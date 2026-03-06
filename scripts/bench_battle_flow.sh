#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOG_BASE_DIR="${LOG_BASE_DIR:-${ROOT_DIR}/build/log}"
FLOW_USERS="${FLOW_USERS:-200}"
CONCURRENCY="${CONCURRENCY:-40}"
ATTACK_STEPS="${ATTACK_STEPS:-6}"
WEBGAME_PORT="${WEBGAME_PORT:-18888}"
WEBGAME_HOST="${WEBGAME_HOST:-127.0.0.1}"
AUTO_START_SERVER="${AUTO_START_SERVER:-1}"
ACCOUNT_PREFIX="${ACCOUNT_PREFIX:-flow_bench}"

URL="http://${WEBGAME_HOST}:${WEBGAME_PORT}/battle"
TIMESTAMP="$(date +%Y%m%d_%H%M%S)"
OUT_DIR="${LOG_BASE_DIR}/battle_flow_bench_${TIMESTAMP}"
TMP_DIR="${OUT_DIR}/tmp"

RAW_METRICS_FILE="${OUT_DIR}/raw_metrics.tsv"
SUMMARY_TXT="${OUT_DIR}/summary.txt"
SUMMARY_JSON="${OUT_DIR}/summary.json"
SERVER_LOG="${OUT_DIR}/server.log"

SERVER_PID=""

cleanup() {
  if [[ -n "${SERVER_PID}" ]] && ps -p "${SERVER_PID}" >/dev/null 2>&1; then
    kill -INT "${SERVER_PID}" >/dev/null 2>&1 || true
    wait "${SERVER_PID}" >/dev/null 2>&1 || true
  fi
}

percentile_from_sorted_file() {
  local sorted_file="$1"
  local percentile="$2"
  local n
  n=$(wc -l < "${sorted_file}" | tr -d ' ')
  if [[ "${n}" -eq 0 ]]; then
    echo "0"
    return
  fi
  local idx
  idx=$(awk -v n="${n}" -v p="${percentile}" 'BEGIN { i = int((n - 1) * p) + 1; if (i < 1) i = 1; if (i > n) i = n; print i }')
  sed -n "${idx}p" "${sorted_file}"
}

mkdir -p "${OUT_DIR}" "${TMP_DIR}"
echo "[bench_battle_flow] output dir: ${OUT_DIR}"
echo "[bench_battle_flow] target: ${URL}"
echo "[bench_battle_flow] users=${FLOW_USERS}, concurrency=${CONCURRENCY}, attack_steps=${ATTACK_STEPS}"

if [[ "${AUTO_START_SERVER}" == "1" ]]; then
  WEBGAME_PORT="${WEBGAME_PORT}" WEBGAME_HOST="${WEBGAME_HOST}" "${ROOT_DIR}/scripts/run_server.sh" > "${SERVER_LOG}" 2>&1 &
  SERVER_PID=$!
  sleep 1
  if ! ps -p "${SERVER_PID}" >/dev/null 2>&1; then
    echo "[bench_battle_flow] failed to start server"
    exit 2
  fi
fi

trap cleanup EXIT

export FLOW_URL="${URL}"
export FLOW_ATTACK_STEPS="${ATTACK_STEPS}"
export FLOW_TMP_DIR="${TMP_DIR}"
export FLOW_ACCOUNT_PREFIX="${ACCOUNT_PREFIX}"

FLOW_START_NS=$(date +%s%N)
seq 1 "${FLOW_USERS}" | xargs -P "${CONCURRENCY}" -I{} bash -c '
id="$1"
account="${FLOW_ACCOUNT_PREFIX}_${id}"
metric_file="${FLOW_TMP_DIR}/metric_${id}.tsv"

start_ns=$(date +%s%N)
ok=1
stage="ok"
http_code=200
attacks_done=0
ended=0
result="ongoing"

start_body=$(curl -sS -X POST "${FLOW_URL}" -H "x-account: ${account}" -H "Content-Type: application/json" -d "{\"action\":\"start\"}" -w "\n%{http_code}")
start_code=$(printf "%s" "${start_body}" | tail -n1)
start_json=$(printf "%s" "${start_body}" | sed "\$d")
if [[ "${start_code}" != "200" ]]; then
  ok=0
  stage="start"
  http_code=${start_code}
fi

battle_id=""
if [[ "${ok}" == "1" ]]; then
  battle_id=$(printf "%s" "${start_json}" | python3 -c "import json,sys; print(json.load(sys.stdin).get(\"session\",{}).get(\"battle_id\",\"\"))" 2>/dev/null || true)
  if [[ -z "${battle_id}" ]]; then
    ok=0
    stage="start_parse"
    http_code=520
  fi
fi

if [[ "${ok}" == "1" ]]; then
  for _ in $(seq 1 "${FLOW_ATTACK_STEPS}"); do
    atk_body=$(curl -sS -X POST "${FLOW_URL}" -H "x-account: ${account}" -H "Content-Type: application/json" -d "{\"action\":\"attack\",\"battle_id\":\"${battle_id}\"}" -w "\n%{http_code}")
    atk_code=$(printf "%s" "${atk_body}" | tail -n1)
    atk_json=$(printf "%s" "${atk_body}" | sed "\$d")
    if [[ "${atk_code}" != "200" ]]; then
      ok=0
      stage="attack"
      http_code=${atk_code}
      break
    fi
    attacks_done=$((attacks_done + 1))
    ended_flag=$(printf "%s" "${atk_json}" | python3 -c "import json,sys; print(\"1\" if json.load(sys.stdin).get(\"session\",{}).get(\"ended\") else \"0\")" 2>/dev/null || echo 0)
    result=$(printf "%s" "${atk_json}" | python3 -c "import json,sys; print(json.load(sys.stdin).get(\"session\",{}).get(\"result\",\"ongoing\"))" 2>/dev/null || echo ongoing)
    if [[ "${ended_flag}" == "1" ]]; then
      ended=1
      break
    fi
  done
fi

if [[ "${ok}" == "1" ]]; then
  end_body=$(curl -sS -X POST "${FLOW_URL}" -H "x-account: ${account}" -H "Content-Type: application/json" -d "{\"action\":\"end\",\"battle_id\":\"${battle_id}\"}" -w "\n%{http_code}")
  end_code=$(printf "%s" "${end_body}" | tail -n1)
  end_json=$(printf "%s" "${end_body}" | sed "\$d")
  if [[ "${end_code}" != "200" ]]; then
    ok=0
    stage="end"
    http_code=${end_code}
  else
    result=$(printf "%s" "${end_json}" | python3 -c "import json,sys; print(json.load(sys.stdin).get(\"session\",{}).get(\"result\",\"ongoing\"))" 2>/dev/null || echo ongoing)
  fi
fi

end_ns=$(date +%s%N)
elapsed_ms=$(( (end_ns - start_ns) / 1000000 ))

echo -e "${id}\t${ok}\t${stage}\t${http_code}\t${elapsed_ms}\t${attacks_done}\t${ended}\t${result}" > "${metric_file}"
' _ {}
FLOW_END_NS=$(date +%s%N)

printf "id\tok\tstage\thttp_code\tflow_elapsed_ms\tattacks_done\tended\tresult\n" > "${RAW_METRICS_FILE}"
find "${TMP_DIR}" -name 'metric_*.tsv' -print0 | sort -z -V | xargs -0 cat >> "${RAW_METRICS_FILE}"

TOTAL=$(awk 'END{print NR-1}' "${RAW_METRICS_FILE}")
SUCCESS=$(awk -F'\t' 'NR>1 && $2==1 {ok++} END{print ok+0}' "${RAW_METRICS_FILE}")
FAILED=$((TOTAL - SUCCESS))
SUCCESS_RATE=$(awk -v ok="${SUCCESS}" -v total="${TOTAL}" 'BEGIN{if(total==0){print "0.00"} else {printf "%.2f", ok*100/total}}')

TOTAL_ELAPSED_MS=$(( (FLOW_END_NS - FLOW_START_NS) / 1000000 ))
FLOW_RPS=$(awk -v total="${TOTAL}" -v ms="${TOTAL_ELAPSED_MS}" 'BEGIN{if(ms==0){print "0.00"} else {printf "%.2f", total*1000/ms}}')

FLOW_LAT_FILE="${OUT_DIR}/_flow_ms_sorted.txt"
awk -F'\t' 'NR>1{print $5}' "${RAW_METRICS_FILE}" | sort -n > "${FLOW_LAT_FILE}"

LAT_MIN=$(head -n1 "${FLOW_LAT_FILE}" 2>/dev/null || echo "0")
LAT_MAX=$(tail -n1 "${FLOW_LAT_FILE}" 2>/dev/null || echo "0")
LAT_AVG=$(awk '{sum+=$1} END{if(NR==0) print "0"; else printf "%.3f", sum/NR}' "${FLOW_LAT_FILE}")
LAT_P50=$(percentile_from_sorted_file "${FLOW_LAT_FILE}" 0.50)
LAT_P90=$(percentile_from_sorted_file "${FLOW_LAT_FILE}" 0.90)
LAT_P95=$(percentile_from_sorted_file "${FLOW_LAT_FILE}" 0.95)
LAT_P99=$(percentile_from_sorted_file "${FLOW_LAT_FILE}" 0.99)

AVG_ATTACKS=$(awk -F'\t' 'NR>1{sum+=$6} END{if(NR<=1) print "0"; else printf "%.2f", sum/(NR-1)}' "${RAW_METRICS_FILE}")
ENDED_RATIO=$(awk -F'\t' 'NR>1{sum+=$7} END{if(NR<=1) print "0.00"; else printf "%.2f", (sum*100)/(NR-1)}' "${RAW_METRICS_FILE}")

STAGE_DISTRIBUTION=$(awk -F'\t' 'NR>1{cnt[$3]++} END{for (k in cnt) printf "%s\t%d\n", k, cnt[k]}' "${RAW_METRICS_FILE}" | sort)
RESULT_DISTRIBUTION=$(awk -F'\t' 'NR>1{cnt[$8]++} END{for (k in cnt) printf "%s\t%d\n", k, cnt[k]}' "${RAW_METRICS_FILE}" | sort)

{
  echo "battle flow benchmark summary"
  echo "timestamp=${TIMESTAMP}"
  echo "url=${URL}"
  echo "flow_users=${FLOW_USERS}"
  echo "concurrency=${CONCURRENCY}"
  echo "attack_steps=${ATTACK_STEPS}"
  echo ""
  echo "throughput"
  echo "  total_flows=${TOTAL}"
  echo "  success_flows=${SUCCESS}"
  echo "  failed_flows=${FAILED}"
  echo "  success_rate_percent=${SUCCESS_RATE}"
  echo "  elapsed_ms=${TOTAL_ELAPSED_MS}"
  echo "  flow_rps=${FLOW_RPS}"
  echo ""
  echo "flow_latency_ms"
  echo "  min=${LAT_MIN}"
  echo "  avg=${LAT_AVG}"
  echo "  p50=${LAT_P50}"
  echo "  p90=${LAT_P90}"
  echo "  p95=${LAT_P95}"
  echo "  p99=${LAT_P99}"
  echo "  max=${LAT_MAX}"
  echo ""
  echo "battle_progress"
  echo "  avg_attacks_done=${AVG_ATTACKS}"
  echo "  ended_within_steps_percent=${ENDED_RATIO}"
  echo ""
  echo "stage_distribution"
  printf "%s\n" "${STAGE_DISTRIBUTION}"
  echo ""
  echo "result_distribution"
  printf "%s\n" "${RESULT_DISTRIBUTION}"
  echo ""
  echo "files"
  echo "  raw_metrics=${RAW_METRICS_FILE}"
  echo "  summary_json=${SUMMARY_JSON}"
  if [[ -n "${SERVER_PID}" ]]; then
    echo "  server_log=${SERVER_LOG}"
  fi
} > "${SUMMARY_TXT}"

STAGE_JSON=$(printf "%s\n" "${STAGE_DISTRIBUTION}" | awk 'BEGIN{first=1} {if($1=="") next; if(!first) printf ","; printf "\"%s\":%s", $1, $2; first=0}')
RESULT_JSON=$(printf "%s\n" "${RESULT_DISTRIBUTION}" | awk 'BEGIN{first=1} {if($1=="") next; if(!first) printf ","; printf "\"%s\":%s", $1, $2; first=0}')

cat > "${SUMMARY_JSON}" <<EOF
{
  "timestamp": "${TIMESTAMP}",
  "url": "${URL}",
  "flow_users": ${FLOW_USERS},
  "concurrency": ${CONCURRENCY},
  "attack_steps": ${ATTACK_STEPS},
  "throughput": {
    "total_flows": ${TOTAL},
    "success_flows": ${SUCCESS},
    "failed_flows": ${FAILED},
    "success_rate_percent": ${SUCCESS_RATE},
    "elapsed_ms": ${TOTAL_ELAPSED_MS},
    "flow_rps": ${FLOW_RPS}
  },
  "flow_latency_ms": {
    "min": ${LAT_MIN},
    "avg": ${LAT_AVG},
    "p50": ${LAT_P50},
    "p90": ${LAT_P90},
    "p95": ${LAT_P95},
    "p99": ${LAT_P99},
    "max": ${LAT_MAX}
  },
  "battle_progress": {
    "avg_attacks_done": ${AVG_ATTACKS},
    "ended_within_steps_percent": ${ENDED_RATIO}
  },
  "stage_distribution": {${STAGE_JSON}},
  "result_distribution": {${RESULT_JSON}},
  "files": {
    "raw_metrics": "${RAW_METRICS_FILE}",
    "summary_txt": "${SUMMARY_TXT}",
    "summary_json": "${SUMMARY_JSON}"
  }
}
EOF

rm -f "${FLOW_LAT_FILE}"

echo "[bench_battle_flow] done"
echo "[bench_battle_flow] summary: ${SUMMARY_TXT}"
echo "[bench_battle_flow] summary json: ${SUMMARY_JSON}"
