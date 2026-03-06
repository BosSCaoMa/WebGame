#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN_PATH="${ROOT_DIR}/build/WebGame"
PORT="${WEBGAME_PORT:-18888}"

port_in_use() {
  local p="$1"
  ss -ltn 2>/dev/null | grep -q ":${p} "
}

find_free_port() {
  local base="$1"
  local p
  for p in $(seq $((base + 1)) $((base + 20))); do
    if ! port_in_use "${p}"; then
      echo "${p}"
      return 0
    fi
  done
  return 1
}

if [[ ! -x "${BIN_PATH}" ]]; then
  echo "[run_server] binary not found: ${BIN_PATH}"
  echo "[run_server] please build first: cmake --build ${ROOT_DIR}/build -j4"
  exit 1
fi

listener_line="$(ss -ltnp 2>/dev/null | grep ":${PORT} " || true)"
if [[ -n "${listener_line}" ]]; then
  echo "[run_server] port ${PORT} is in use"
  pids="$(pgrep -af WebGame | awk '{print $1}' | xargs || true)"
  if [[ -n "${pids}" ]]; then
    echo "[run_server] found WebGame process: ${pids}, stopping..."
    kill -INT ${pids} || true
    sleep 1
  fi

  if port_in_use "${PORT}"; then
    echo "[run_server] port ${PORT} still in use by other process"
    alt_port="$(find_free_port "${PORT}" || true)"
    if [[ -z "${alt_port}" ]]; then
      echo "[run_server] no free port found in [$((PORT + 1)), $((PORT + 20))], abort"
      ss -ltnp 2>/dev/null | grep ":${PORT} " || true
      exit 2
    fi
    PORT="${alt_port}"
    export WEBGAME_PORT="${PORT}"
    echo "[run_server] fallback to WEBGAME_PORT=${PORT}"
  fi
fi

echo "[run_server] starting ${BIN_PATH} on port ${PORT}"
exec "${BIN_PATH}"
