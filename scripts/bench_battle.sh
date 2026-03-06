#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BENCH_ROOT_DIR="${BENCH_ROOT_DIR:-${ROOT_DIR}/build/benchmarks}"
LOG_BASE_DIR="${LOG_BASE_DIR:-${BENCH_ROOT_DIR}/battle_api}"
TOTAL_REQUESTS="${TOTAL_REQUESTS:-500}"
CONCURRENCY="${CONCURRENCY:-50}"
WEBGAME_PORT="${WEBGAME_PORT:-18888}"
WEBGAME_HOST="${WEBGAME_HOST:-127.0.0.1}"
AUTO_START_SERVER="${AUTO_START_SERVER:-1}"
BATTLE_ACTION="${BATTLE_ACTION:-attack}"
BATTLE_ACCOUNT="${BATTLE_ACCOUNT:-bench_user}"

URL="http://${WEBGAME_HOST}:${WEBGAME_PORT}/battle"
TIMESTAMP="$(date +%Y%m%d_%H%M%S)"
OUT_DIR="${LOG_BASE_DIR}/battle_bench_${TIMESTAMP}"
TMP_DIR="${OUT_DIR}/tmp"

RAW_METRICS_FILE="${OUT_DIR}/raw_metrics.tsv"
STATUS_COUNT_FILE="${OUT_DIR}/status_counts.tsv"
SUMMARY_TXT="${OUT_DIR}/summary.txt"
SUMMARY_JSON="${OUT_DIR}/summary.json"
SERVER_LOG="${OUT_DIR}/server.log"

SERVER_PID=""

require_cmd() {
  local cmd="$1"
  if ! command -v "${cmd}" >/dev/null 2>&1; then
    echo "[bench_battle] missing command: ${cmd}"
    exit 1
  fi
}

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

require_cmd curl
require_cmd awk
require_cmd sort
require_cmd xargs
require_cmd seq
require_cmd sed

mkdir -p "${OUT_DIR}" "${TMP_DIR}"

echo "[bench_battle] output dir: ${OUT_DIR}"
echo "[bench_battle] target: ${URL}"
echo "[bench_battle] total=${TOTAL_REQUESTS}, concurrency=${CONCURRENCY}"

if [[ "${AUTO_START_SERVER}" == "1" ]]; then
  WEBGAME_PORT="${WEBGAME_PORT}" WEBGAME_HOST="${WEBGAME_HOST}" "${ROOT_DIR}/scripts/run_server.sh" > "${SERVER_LOG}" 2>&1 &
  SERVER_PID=$!
  sleep 1
  if ! ps -p "${SERVER_PID}" >/dev/null 2>&1; then
    echo "[bench_battle] failed to start server"
    exit 2
  fi
else
  echo "[bench_battle] AUTO_START_SERVER=0, assume server already running"
fi

trap cleanup EXIT

export BENCH_URL="${URL}"
export BENCH_ACTION="${BATTLE_ACTION}"
export BENCH_ACCOUNT="${BATTLE_ACCOUNT}"
export BENCH_TMP_DIR="${TMP_DIR}"

START_NS=$(date +%s%N)
seq 1 "${TOTAL_REQUESTS}" | xargs -P "${CONCURRENCY}" -I{} bash -c '
id="$1"
payload="{\"action\":\"${BENCH_ACTION}\"}"
body_file="${BENCH_TMP_DIR}/resp_${id}.txt"
metric_file="${BENCH_TMP_DIR}/metric_${id}.tsv"

set +e
metrics=$(curl -sS -o "${body_file}" \
  -H "x-account: ${BENCH_ACCOUNT}" \
  -H "Content-Type: application/json" \
  -X POST "${BENCH_URL}" \
  -d "${payload}" \
  -w "%{http_code}\t%{time_total}\t%{time_connect}\t%{time_starttransfer}\t%{size_download}\t%{num_connects}\t%{remote_ip}\t%{remote_port}\t%{errormsg}")
curl_exit=$?
set -e

if [[ -z "${metrics}" ]]; then
  metrics="000\t0\t0\t0\t0\t0\t-\t0\tempty_metrics"
fi

echo -e "${id}\t${curl_exit}\t${metrics}" > "${metric_file}"
' _ {}
END_NS=$(date +%s%N)

printf "id\tcurl_exit\thttp_code\ttime_total_s\ttime_connect_s\ttime_ttfb_s\tsize_download\tnum_connects\tremote_ip\tremote_port\terrmsg\n" > "${RAW_METRICS_FILE}"
find "${TMP_DIR}" -name 'metric_*.tsv' -print0 | sort -z -V | xargs -0 cat >> "${RAW_METRICS_FILE}"

awk -F'\t' 'NR>1{cnt[$3]++} END{for (code in cnt) printf "%s\t%d\n", code, cnt[code]}' "${RAW_METRICS_FILE}" | sort -k1,1n > "${STATUS_COUNT_FILE}"

TOTAL_DONE=$(awk 'END{print NR-1}' "${RAW_METRICS_FILE}")
SUCCESS_COUNT=$(awk -F'\t' 'NR>1 && $2==0 && $3=="200" {ok++} END{print ok+0}' "${RAW_METRICS_FILE}")
FAIL_COUNT=$((TOTAL_DONE - SUCCESS_COUNT))
SUCCESS_RATE=$(awk -v ok="${SUCCESS_COUNT}" -v total="${TOTAL_DONE}" 'BEGIN{ if(total==0){print "0.00"} else {printf "%.2f", ok*100/total} }')

ELAPSED_MS=$(( (END_NS - START_NS) / 1000000 ))
RPS=$(awk -v total="${TOTAL_DONE}" -v ms="${ELAPSED_MS}" 'BEGIN{ if(ms==0){print "0.00"} else {printf "%.2f", total*1000/ms} }')

TOTAL_MS_SORTED="${OUT_DIR}/_total_ms_sorted.txt"
CONNECT_MS_SORTED="${OUT_DIR}/_connect_ms_sorted.txt"
TTFB_MS_SORTED="${OUT_DIR}/_ttfb_ms_sorted.txt"

awk -F'\t' 'NR>1{printf "%.3f\n", $4*1000}' "${RAW_METRICS_FILE}" | sort -n > "${TOTAL_MS_SORTED}"
awk -F'\t' 'NR>1{printf "%.3f\n", $5*1000}' "${RAW_METRICS_FILE}" | sort -n > "${CONNECT_MS_SORTED}"
awk -F'\t' 'NR>1{printf "%.3f\n", $6*1000}' "${RAW_METRICS_FILE}" | sort -n > "${TTFB_MS_SORTED}"

LAT_MIN=$(head -n1 "${TOTAL_MS_SORTED}" 2>/dev/null || echo "0")
LAT_MAX=$(tail -n1 "${TOTAL_MS_SORTED}" 2>/dev/null || echo "0")
LAT_AVG=$(awk '{sum+=$1} END{if(NR==0) print "0"; else printf "%.3f", sum/NR}' "${TOTAL_MS_SORTED}")
LAT_P50=$(percentile_from_sorted_file "${TOTAL_MS_SORTED}" 0.50)
LAT_P90=$(percentile_from_sorted_file "${TOTAL_MS_SORTED}" 0.90)
LAT_P95=$(percentile_from_sorted_file "${TOTAL_MS_SORTED}" 0.95)
LAT_P99=$(percentile_from_sorted_file "${TOTAL_MS_SORTED}" 0.99)

CONNECT_AVG=$(awk '{sum+=$1} END{if(NR==0) print "0"; else printf "%.3f", sum/NR}' "${CONNECT_MS_SORTED}")
CONNECT_P95=$(percentile_from_sorted_file "${CONNECT_MS_SORTED}" 0.95)

TTFB_AVG=$(awk '{sum+=$1} END{if(NR==0) print "0"; else printf "%.3f", sum/NR}' "${TTFB_MS_SORTED}")
TTFB_P95=$(percentile_from_sorted_file "${TTFB_MS_SORTED}" 0.95)

AVG_DOWNLOAD_BYTES=$(awk -F'\t' 'NR>1{sum+=$7} END{if(NR<=1) print "0"; else printf "%.2f", sum/(NR-1)}' "${RAW_METRICS_FILE}")

{
  echo "battle benchmark summary"
  echo "timestamp=${TIMESTAMP}"
  echo "url=${URL}"
  echo "total_requests=${TOTAL_REQUESTS}"
  echo "concurrency=${CONCURRENCY}"
  echo "battle_action=${BATTLE_ACTION}"
  echo "battle_account=${BATTLE_ACCOUNT}"
  echo ""
  echo "throughput"
  echo "  total_done=${TOTAL_DONE}"
  echo "  success=${SUCCESS_COUNT}"
  echo "  fail=${FAIL_COUNT}"
  echo "  success_rate_percent=${SUCCESS_RATE}"
  echo "  elapsed_ms=${ELAPSED_MS}"
  echo "  rps=${RPS}"
  echo ""
  echo "latency_total_ms"
  echo "  min=${LAT_MIN}"
  echo "  avg=${LAT_AVG}"
  echo "  p50=${LAT_P50}"
  echo "  p90=${LAT_P90}"
  echo "  p95=${LAT_P95}"
  echo "  p99=${LAT_P99}"
  echo "  max=${LAT_MAX}"
  echo ""
  echo "network_phase_ms"
  echo "  connect_avg=${CONNECT_AVG}"
  echo "  connect_p95=${CONNECT_P95}"
  echo "  ttfb_avg=${TTFB_AVG}"
  echo "  ttfb_p95=${TTFB_P95}"
  echo ""
  echo "payload"
  echo "  avg_response_bytes=${AVG_DOWNLOAD_BYTES}"
  echo ""
  echo "status_code_distribution"
  cat "${STATUS_COUNT_FILE}"
  echo ""
  echo "files"
  echo "  raw_metrics=${RAW_METRICS_FILE}"
  echo "  status_counts=${STATUS_COUNT_FILE}"
  echo "  summary_json=${SUMMARY_JSON}"
  if [[ -n "${SERVER_PID}" ]]; then
    echo "  server_log=${SERVER_LOG}"
  fi
} > "${SUMMARY_TXT}"

STATUS_JSON=$(awk 'BEGIN{first=1} {if(!first) printf ","; printf "\"%s\":%s", $1, $2; first=0}' "${STATUS_COUNT_FILE}")
if [[ -z "${STATUS_JSON}" ]]; then
  STATUS_JSON=""
fi

cat > "${SUMMARY_JSON}" <<EOF
{
  "timestamp": "${TIMESTAMP}",
  "url": "${URL}",
  "total_requests": ${TOTAL_REQUESTS},
  "concurrency": ${CONCURRENCY},
  "action": "${BATTLE_ACTION}",
  "account": "${BATTLE_ACCOUNT}",
  "throughput": {
    "total_done": ${TOTAL_DONE},
    "success": ${SUCCESS_COUNT},
    "fail": ${FAIL_COUNT},
    "success_rate_percent": ${SUCCESS_RATE},
    "elapsed_ms": ${ELAPSED_MS},
    "rps": ${RPS}
  },
  "latency_total_ms": {
    "min": ${LAT_MIN},
    "avg": ${LAT_AVG},
    "p50": ${LAT_P50},
    "p90": ${LAT_P90},
    "p95": ${LAT_P95},
    "p99": ${LAT_P99},
    "max": ${LAT_MAX}
  },
  "network_phase_ms": {
    "connect_avg": ${CONNECT_AVG},
    "connect_p95": ${CONNECT_P95},
    "ttfb_avg": ${TTFB_AVG},
    "ttfb_p95": ${TTFB_P95}
  },
  "payload": {
    "avg_response_bytes": ${AVG_DOWNLOAD_BYTES}
  },
  "status_code_distribution": {${STATUS_JSON}},
  "files": {
    "raw_metrics": "${RAW_METRICS_FILE}",
    "status_counts": "${STATUS_COUNT_FILE}",
    "summary_txt": "${SUMMARY_TXT}",
    "summary_json": "${SUMMARY_JSON}"
  }
}
EOF

rm -f "${TOTAL_MS_SORTED}" "${CONNECT_MS_SORTED}" "${TTFB_MS_SORTED}"

echo "[bench_battle] done"
echo "[bench_battle] summary: ${SUMMARY_TXT}"
echo "[bench_battle] summary json: ${SUMMARY_JSON}"
