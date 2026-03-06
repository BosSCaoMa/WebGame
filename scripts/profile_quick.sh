#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BENCH_ROOT_DIR="${BENCH_ROOT_DIR:-${ROOT_DIR}/test/benchmarks}"
OUT_DIR="${BENCH_ROOT_DIR}/profiling_$(date +%Y%m%d_%H%M%S)"

DURATION_SEC="${DURATION_SEC:-20}"
SAMPLE_INTERVAL_SEC="${SAMPLE_INTERVAL_SEC:-1}"
RUN_MATRIX="${RUN_MATRIX:-1}"
MATRIX_POINTS="${MATRIX_POINTS:-400:40,800:80,1200:120}"
FLOW_MODE="${FLOW_MODE:-auto}"
AUTO_START_SERVER="${AUTO_START_SERVER:-1}"
TARGET_PID="${TARGET_PID:-}"
WARMUP_SEC="${WARMUP_SEC:-2}"

MATRIX_LOG="${OUT_DIR}/matrix.log"
PS_THREADS_FILE="${OUT_DIR}/ps_threads.txt"
TOP_THREADS_FILE="${OUT_DIR}/top_threads.txt"
PIDSTAT_U_FILE="${OUT_DIR}/pidstat_u.txt"
PIDSTAT_W_FILE="${OUT_DIR}/pidstat_w.txt"
VMSTAT_FILE="${OUT_DIR}/vmstat.txt"
STRACE_SUMMARY_FILE="${OUT_DIR}/strace_summary.txt"
PERF_REPORT_FILE="${OUT_DIR}/perf_report.txt"
PERF_DATA_FILE="${OUT_DIR}/perf.data"
SUMMARY_TXT="${OUT_DIR}/summary.txt"
META_JSON="${OUT_DIR}/meta.json"

mkdir -p "${OUT_DIR}"

log() {
  echo "[profile_quick] $*"
}

have_cmd() {
  command -v "$1" >/dev/null 2>&1
}

safe_run_bg() {
  local cmd="$1"
  local out="$2"
  bash -lc "$cmd" >"$out" 2>&1 &
  echo $!
}

MATRIX_JOB_PID=""
if [[ "${RUN_MATRIX}" == "1" ]]; then
  log "starting matrix benchmark in background"
  (
    cd "${ROOT_DIR}"
    MATRIX_POINTS="${MATRIX_POINTS}" \
    FLOW_MODE="${FLOW_MODE}" \
    AUTO_START_SERVER="${AUTO_START_SERVER}" \
    BENCH_ROOT_DIR="${BENCH_ROOT_DIR}" \
    "${ROOT_DIR}/scripts/run_bench_matrix.sh"
  ) >"${MATRIX_LOG}" 2>&1 &
  MATRIX_JOB_PID=$!
  sleep "${WARMUP_SEC}"
fi

if [[ -z "${TARGET_PID}" ]]; then
  for _ in $(seq 1 20); do
    TARGET_PID="$(pgrep -n WebGame || true)"
    if [[ -n "${TARGET_PID}" ]]; then
      break
    fi
    sleep 1
  done
fi

if [[ -z "${TARGET_PID}" ]]; then
  log "cannot find WebGame pid, abort"
  if [[ -n "${MATRIX_JOB_PID}" ]]; then
    wait "${MATRIX_JOB_PID}" || true
  fi
  exit 2
fi

log "target pid=${TARGET_PID}"

# thread snapshot
if have_cmd ps; then
  ps -L -p "${TARGET_PID}" -o pid,tid,pcpu,comm --sort=-pcpu > "${PS_THREADS_FILE}" || true
fi
if have_cmd top; then
  top -H -b -n 1 -p "${TARGET_PID}" > "${TOP_THREADS_FILE}" || true
fi

PIDS_TO_WAIT=()

if have_cmd pidstat; then
  pidstat -u -t -p "${TARGET_PID}" "${SAMPLE_INTERVAL_SEC}" "${DURATION_SEC}" > "${PIDSTAT_U_FILE}" 2>&1 &
  PIDS_TO_WAIT+=("$!")
  pidstat -w -t -p "${TARGET_PID}" "${SAMPLE_INTERVAL_SEC}" "${DURATION_SEC}" > "${PIDSTAT_W_FILE}" 2>&1 &
  PIDS_TO_WAIT+=("$!")
fi

if have_cmd vmstat; then
  vmstat "${SAMPLE_INTERVAL_SEC}" "${DURATION_SEC}" > "${VMSTAT_FILE}" 2>&1 &
  PIDS_TO_WAIT+=("$!")
fi

if have_cmd strace; then
  timeout "${DURATION_SEC}s" strace -f -p "${TARGET_PID}" -c -qq \
    -e trace=network,futex,epoll_wait,poll > "${STRACE_SUMMARY_FILE}" 2>&1 || true
fi

if have_cmd perf; then
  if timeout "${DURATION_SEC}s" perf record -F 99 -g -p "${TARGET_PID}" -o "${PERF_DATA_FILE}" -- sleep "${DURATION_SEC}" > /dev/null 2>&1; then
    perf report --stdio -i "${PERF_DATA_FILE}" > "${PERF_REPORT_FILE}" 2>&1 || true
  else
    echo "perf record failed (likely permission/perf_event_paranoid)" > "${PERF_REPORT_FILE}"
  fi
fi

for job in "${PIDS_TO_WAIT[@]:-}"; do
  if [[ -n "${job}" ]]; then
    wait "${job}" || true
  fi
done

MATRIX_EXIT=0
if [[ -n "${MATRIX_JOB_PID}" ]]; then
  wait "${MATRIX_JOB_PID}" || MATRIX_EXIT=$?
fi

python3 - <<'PY' \
"${OUT_DIR}" "${TARGET_PID}" "${DURATION_SEC}" "${RUN_MATRIX}" "${MATRIX_EXIT}" \
"${MATRIX_LOG}" "${PS_THREADS_FILE}" "${PIDSTAT_W_FILE}" "${VMSTAT_FILE}" "${STRACE_SUMMARY_FILE}" "${PERF_REPORT_FILE}" "${SUMMARY_TXT}" "${META_JSON}"
import json, os, re, sys
(
    out_dir, target_pid, duration_sec, run_matrix, matrix_exit,
    matrix_log, ps_threads_file, pidstat_w_file, vmstat_file,
    strace_summary_file, perf_report_file, summary_txt, meta_json
) = sys.argv[1:]

result = {
    "target_pid": int(target_pid),
    "duration_sec": int(duration_sec),
    "run_matrix": run_matrix == "1",
    "matrix_exit": int(matrix_exit),
    "artifacts": {
        "out_dir": out_dir,
        "matrix_log": matrix_log if os.path.exists(matrix_log) else None,
        "ps_threads": ps_threads_file if os.path.exists(ps_threads_file) else None,
        "pidstat_w": pidstat_w_file if os.path.exists(pidstat_w_file) else None,
        "vmstat": vmstat_file if os.path.exists(vmstat_file) else None,
        "strace_summary": strace_summary_file if os.path.exists(strace_summary_file) else None,
        "perf_report": perf_report_file if os.path.exists(perf_report_file) else None,
    },
    "signals": {}
}

max_thread_cpu = None
if os.path.exists(ps_threads_file):
    with open(ps_threads_file, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()[1:]
    cpus = []
    for line in lines:
        cols = line.split()
        if len(cols) >= 4:
            try:
                cpus.append(float(cols[2]))
            except ValueError:
                pass
    if cpus:
        max_thread_cpu = max(cpus)
        result["signals"]["max_thread_cpu_percent"] = max_thread_cpu

avg_cswch = None
if os.path.exists(pidstat_w_file):
    nums = []
    with open(pidstat_w_file, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            if "Average:" in line:
                cols = line.split()
                if len(cols) >= 6:
                    try:
                        cswch = float(cols[4])
                        nvcswch = float(cols[5])
                        nums.append(cswch + nvcswch)
                    except ValueError:
                        pass
    if nums:
        avg_cswch = sum(nums) / len(nums)
        result["signals"]["avg_context_switch_per_sec"] = avg_cswch

max_runq = None
if os.path.exists(vmstat_file):
    runqs = []
    with open(vmstat_file, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            cols = line.split()
            if len(cols) >= 2 and cols[0].isdigit() and cols[1].isdigit():
                try:
                    runqs.append(int(cols[0]))
                except ValueError:
                    pass
    if runqs:
        max_runq = max(runqs)
        result["signals"]["max_run_queue"] = max_runq

strace_pct = {}
if os.path.exists(strace_summary_file):
    with open(strace_summary_file, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            m = re.match(r"\s*([0-9]+\.?[0-9]*)\s+.*\s+([a-zA-Z0-9_]+)\s*$", line)
            if m:
                pct = float(m.group(1))
                syscall = m.group(2)
                strace_pct[syscall] = pct
if strace_pct:
    result["signals"]["strace_percent"] = strace_pct

futex = strace_pct.get("futex", 0.0)
epoll = strace_pct.get("epoll_wait", 0.0)
poll = strace_pct.get("poll", 0.0)

diagnosis = []
if futex >= 30:
    diagnosis.append("可能存在锁争用（futex 占比较高）")
if (epoll + poll) >= 50:
    diagnosis.append("线程大量等待事件（epoll_wait/poll 占比较高）")
if max_thread_cpu is not None and max_thread_cpu >= 80:
    diagnosis.append("可能存在单线程热点")
if max_runq is not None and max_runq >= 4:
    diagnosis.append("运行队列较高，可能有调度竞争")
if not diagnosis:
    diagnosis.append("未发现明显单一瓶颈信号，建议结合 perf 热点与业务日志继续定位")

matrix_summary_path = None
if os.path.exists(matrix_log):
    with open(matrix_log, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            if "summary json:" in line:
                matrix_summary_path = line.strip().split("summary json:", 1)[-1].strip()

lines = []
lines.append("quick profiling summary")
lines.append(f"target_pid={target_pid}")
lines.append(f"duration_sec={duration_sec}")
lines.append(f"run_matrix={run_matrix}")
lines.append(f"matrix_exit={matrix_exit}")
lines.append("")
lines.append("signals")
if max_thread_cpu is not None:
    lines.append(f"  max_thread_cpu_percent={max_thread_cpu:.2f}")
if avg_cswch is not None:
    lines.append(f"  avg_context_switch_per_sec={avg_cswch:.2f}")
if max_runq is not None:
    lines.append(f"  max_run_queue={max_runq}")
if strace_pct:
    top_syscalls = sorted(strace_pct.items(), key=lambda x: x[1], reverse=True)[:5]
    for name, pct in top_syscalls:
        lines.append(f"  strace_{name}_percent={pct:.2f}")
lines.append("")
lines.append("diagnosis")
for d in diagnosis:
    lines.append(f"  - {d}")
lines.append("")
lines.append("files")
lines.append(f"  out_dir={out_dir}")
if matrix_summary_path:
    lines.append(f"  matrix_summary_json={matrix_summary_path}")
for key, val in result["artifacts"].items():
    if key == "out_dir":
        continue
    if val:
        lines.append(f"  {key}={val}")

if os.path.exists(perf_report_file):
    lines.append("")
    lines.append("perf_top (first matches)")
    with open(perf_report_file, "r", encoding="utf-8", errors="ignore") as f:
        count = 0
        for line in f:
            if re.match(r"\s*[0-9]+\.[0-9]+%", line):
                lines.append("  " + line.rstrip())
                count += 1
                if count >= 8:
                    break

with open(summary_txt, "w", encoding="utf-8") as f:
    f.write("\n".join(lines) + "\n")

result["diagnosis"] = diagnosis
result["matrix_summary_json"] = matrix_summary_path
with open(meta_json, "w", encoding="utf-8") as f:
    json.dump(result, f, ensure_ascii=False, indent=2)
PY

log "done"
log "summary: ${SUMMARY_TXT}"
log "meta: ${META_JSON}"
