#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
FLOW_BENCH_SCRIPT="${ROOT_DIR}/scripts/bench_battle_flow.sh"
SUMMARIZE_SCRIPT="${ROOT_DIR}/scripts/summarize_battle_flow.py"

BENCH_ROOT_DIR="${BENCH_ROOT_DIR:-${ROOT_DIR}/test/benchmarks}"
LOG_BASE_DIR="${LOG_BASE_DIR:-${BENCH_ROOT_DIR}/battle_flow}"
FLOW_MODE="${FLOW_MODE:-auto}"
AUTO_START_SERVER="${AUTO_START_SERVER:-1}"
STOP_ON_FAIL="${STOP_ON_FAIL:-0}"

MATRIX_POINTS="${MATRIX_POINTS:-400:40,800:80,1200:120,1600:160,2400:320,3200:480}"

TIMESTAMP="$(date +%Y%m%d_%H%M%S)"
MATRIX_DIR="${LOG_BASE_DIR}/matrix_${TIMESTAMP}"
MATRIX_RAW_TSV="${MATRIX_DIR}/matrix_raw.tsv"
MATRIX_SUMMARY_TXT="${MATRIX_DIR}/matrix_summary.txt"
MATRIX_SUMMARY_JSON="${MATRIX_DIR}/matrix_summary.json"

mkdir -p "${MATRIX_DIR}"

if [[ ! -x "${FLOW_BENCH_SCRIPT}" ]]; then
  echo "[run_bench_matrix] missing executable script: ${FLOW_BENCH_SCRIPT}"
  echo "[run_bench_matrix] try: chmod +x ${FLOW_BENCH_SCRIPT}"
  exit 1
fi

echo "[run_bench_matrix] matrix points: ${MATRIX_POINTS}"
echo "[run_bench_matrix] flow mode: ${FLOW_MODE}"
echo "[run_bench_matrix] log base: ${LOG_BASE_DIR}"
echo "[run_bench_matrix] matrix dir: ${MATRIX_DIR}"

printf "users\tconcurrency\texit_code\tsuccess_rate_percent\tflow_rps\tp95_ms\tavg_ms\tfailed_flows\tresult_dir\n" > "${MATRIX_RAW_TSV}"

IFS=',' read -r -a points <<< "${MATRIX_POINTS}"

for point in "${points[@]}"; do
  users="${point%%:*}"
  concurrency="${point##*:}"

  if [[ -z "${users}" || -z "${concurrency}" || "${users}" == "${concurrency}" ]]; then
    echo "[run_bench_matrix] skip invalid point: ${point}"
    continue
  fi

  run_log="${MATRIX_DIR}/run_${users}_${concurrency}.log"
  echo "[run_bench_matrix] start users=${users} concurrency=${concurrency}"

  set +e
  FLOW_USERS="${users}" \
  CONCURRENCY="${concurrency}" \
  FLOW_MODE="${FLOW_MODE}" \
  AUTO_START_SERVER="${AUTO_START_SERVER}" \
  BENCH_ROOT_DIR="${BENCH_ROOT_DIR}" \
  LOG_BASE_DIR="${LOG_BASE_DIR}" \
  WEBGAME_PORT="${WEBGAME_PORT:-18888}" \
  WEBGAME_HOST="${WEBGAME_HOST:-127.0.0.1}" \
  ATTACK_STEPS="${ATTACK_STEPS:-6}" \
  ACCOUNT_PREFIX="${ACCOUNT_PREFIX:-flow_bench}" \
  "${FLOW_BENCH_SCRIPT}" > "${run_log}" 2>&1
  exit_code=$?
  set -e

  result_dir="$(grep -m1 'output dir:' "${run_log}" | awk '{print $NF}' || true)"
  summary_json=""
  if [[ -n "${result_dir}" ]]; then
    summary_json="${result_dir}/summary.json"
  fi

  if [[ ${exit_code} -eq 0 && -n "${summary_json}" && -f "${summary_json}" ]]; then
    row="$(python3 - <<'PY' "${summary_json}" "${users}" "${concurrency}" "${result_dir}"
import json,sys
path,users,conc,result_dir=sys.argv[1:5]
with open(path,'r',encoding='utf-8') as f:
    d=json.load(f)
t=d.get('throughput',{})
l=d.get('flow_latency_ms',{})
print(f"{users}\t{conc}\t0\t{t.get('success_rate_percent',0)}\t{t.get('flow_rps',0)}\t{l.get('p95',0)}\t{l.get('avg',0)}\t{t.get('failed_flows',0)}\t{result_dir}")
PY
)"
    printf "%s\n" "${row}" >> "${MATRIX_RAW_TSV}"
    echo "[run_bench_matrix] done users=${users} concurrency=${concurrency}"
  else
    printf "%s\t%s\t%s\t0\t0\t0\t0\t-1\t%s\n" "${users}" "${concurrency}" "${exit_code}" "${result_dir:-N/A}" >> "${MATRIX_RAW_TSV}"
    echo "[run_bench_matrix] failed users=${users} concurrency=${concurrency} exit_code=${exit_code}"
    if [[ "${STOP_ON_FAIL}" == "1" ]]; then
      echo "[run_bench_matrix] STOP_ON_FAIL=1, abort"
      break
    fi
  fi

done

if command -v python3 >/dev/null 2>&1; then
  if [[ -f "${SUMMARIZE_SCRIPT}" ]]; then
    python3 "${SUMMARIZE_SCRIPT}" \
      --base-dir "${LOG_BASE_DIR}" \
      --flow-mode "${FLOW_MODE}" >/dev/null || true
  fi

  python3 - <<'PY' "${MATRIX_RAW_TSV}" "${MATRIX_SUMMARY_TXT}" "${MATRIX_SUMMARY_JSON}" "${FLOW_MODE}" "${MATRIX_POINTS}"
import csv,json,sys
raw_tsv,txt_out,json_out,flow_mode,points=sys.argv[1:6]
rows=[]
with open(raw_tsv,'r',encoding='utf-8') as f:
    reader=csv.DictReader(f,delimiter='\t')
    for r in reader:
        rows.append(r)

ok_rows=[]
for r in rows:
    try:
        if int(float(r['exit_code']))==0:
            ok_rows.append(r)
    except Exception:
        pass

def f(v,default=0.0):
    try:
        return float(v)
    except Exception:
        return default

peak=max(ok_rows,key=lambda r:f(r['flow_rps'])) if ok_rows else None
stable=max((r for r in ok_rows if f(r['success_rate_percent'])>=99.9),key=lambda r:f(r['flow_rps']),default=None)
lowlat=max((r for r in ok_rows if f(r['success_rate_percent'])>=99.9 and f(r['p95_ms'])<=10000),key=lambda r:f(r['flow_rps']),default=None)

lines=[]
lines.append('battle_flow matrix summary')
lines.append(f'flow_mode={flow_mode}')
lines.append(f'matrix_points={points}')
lines.append('')
lines.append('users\tconcurrency\texit_code\tsuccess_rate_percent\tflow_rps\tp95_ms\tavg_ms\tfailed_flows\tresult_dir')
for r in rows:
    lines.append('\t'.join([
        r.get('users',''),r.get('concurrency',''),r.get('exit_code',''),
        r.get('success_rate_percent',''),r.get('flow_rps',''),r.get('p95_ms',''),
        r.get('avg_ms',''),r.get('failed_flows',''),r.get('result_dir','')
    ]))

def fmt(name,row):
    lines.append('')
    lines.append(name)
    if row is None:
        lines.append('none')
    else:
        lines.append(
            f"users={row['users']}, conc={row['concurrency']}, success={row['success_rate_percent']}%, "
            f"rps={row['flow_rps']}, p95={row['p95_ms']}ms, avg={row['avg_ms']}ms, failed={row['failed_flows']}, dir={row['result_dir']}"
        )

fmt('peak',peak)
fmt('stable',stable)
fmt('lowlat',lowlat)

with open(txt_out,'w',encoding='utf-8') as f:
    f.write('\n'.join(lines)+'\n')

with open(json_out,'w',encoding='utf-8') as f:
    json.dump({
        'flow_mode':flow_mode,
        'matrix_points':points,
        'rows':rows,
        'peak':peak,
        'stable':stable,
        'lowlat':lowlat,
    },f,ensure_ascii=False,indent=2)
PY
fi

echo "[run_bench_matrix] raw: ${MATRIX_RAW_TSV}"
echo "[run_bench_matrix] summary: ${MATRIX_SUMMARY_TXT}"
echo "[run_bench_matrix] summary json: ${MATRIX_SUMMARY_JSON}"
