#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MATRIX_SCRIPT="${ROOT_DIR}/scripts/run_bench_matrix.sh"

SERVER_HOST="${SERVER_HOST:-${WEBGAME_HOST:-}}"
SERVER_PORT="${SERVER_PORT:-${WEBGAME_PORT:-18888}}"
MATRIX_POINTS="${MATRIX_POINTS:-400:40,800:80,1200:120,1600:160,2400:320,3200:480}"
FLOW_MODE="${FLOW_MODE:-auto}"
BENCH_ROOT_DIR="${BENCH_ROOT_DIR:-${ROOT_DIR}/test/benchmarks_remote}"
STOP_ON_FAIL="${STOP_ON_FAIL:-0}"
PING_PATH="${PING_PATH:-/ping}"

if [[ -z "${SERVER_HOST}" ]]; then
  echo "[run_remote_bench_matrix] missing SERVER_HOST"
  echo "[run_remote_bench_matrix] example: SERVER_HOST=10.0.0.8 SERVER_PORT=18888 ./scripts/run_remote_bench_matrix.sh"
  exit 1
fi

if [[ ! -x "${MATRIX_SCRIPT}" ]]; then
  echo "[run_remote_bench_matrix] matrix script not executable: ${MATRIX_SCRIPT}"
  echo "[run_remote_bench_matrix] run: chmod +x ${MATRIX_SCRIPT}"
  exit 1
fi

TARGET_URL="http://${SERVER_HOST}:${SERVER_PORT}${PING_PATH}"
echo "[run_remote_bench_matrix] target=${TARGET_URL}"
echo "[run_remote_bench_matrix] matrix_points=${MATRIX_POINTS}"
echo "[run_remote_bench_matrix] bench_root=${BENCH_ROOT_DIR}"

if command -v curl >/dev/null 2>&1; then
  if ! curl -fsS --max-time 3 "${TARGET_URL}" >/dev/null; then
    echo "[run_remote_bench_matrix] health check failed: ${TARGET_URL}"
    echo "[run_remote_bench_matrix] make sure server is running and accessible from this machine"
    exit 2
  fi
else
  echo "[run_remote_bench_matrix] warning: curl not found, skip health check"
fi

cd "${ROOT_DIR}"
AUTO_START_SERVER=0 \
WEBGAME_HOST="${SERVER_HOST}" \
WEBGAME_PORT="${SERVER_PORT}" \
FLOW_MODE="${FLOW_MODE}" \
MATRIX_POINTS="${MATRIX_POINTS}" \
BENCH_ROOT_DIR="${BENCH_ROOT_DIR}" \
STOP_ON_FAIL="${STOP_ON_FAIL}" \
./scripts/run_bench_matrix.sh

echo "[run_remote_bench_matrix] done"
echo "[run_remote_bench_matrix] results under ${BENCH_ROOT_DIR}/battle_flow"
