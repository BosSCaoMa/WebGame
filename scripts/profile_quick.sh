#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BENCH_ROOT_DIR="${BENCH_ROOT_DIR:-${ROOT_DIR}/test/benchmarks}"
OUT_DIR="${BENCH_ROOT_DIR}/profiling_$(date +%Y%m%d_%H%M%S)"

PROFILE_MODE="${PROFILE_MODE:-interactive}" # interactive | timed
DURATION_SEC="${DURATION_SEC:-20}"
SAMPLE_INTERVAL_SEC="${SAMPLE_INTERVAL_SEC:-1}"
RUN_MATRIX="${RUN_MATRIX:-1}"
MATRIX_POINTS="${MATRIX_POINTS:-400:40,800:80,1200:120}"
FLOW_MODE="${FLOW_MODE:-auto}"
AUTO_START_SERVER="${AUTO_START_SERVER:-1}"
TARGET_PID="${TARGET_PID:-}"
WARMUP_SEC="${WARMUP_SEC:-2}"
START_SERVER="${START_SERVER:-1}"
STOP_COMMAND="${STOP_COMMAND:-stop}"
WEBGAME_BIN="${WEBGAME_BIN:-${ROOT_DIR}/build/WebGame}"

MATRIX_LOG="${OUT_DIR}/matrix.log"
SERVER_LOG="${OUT_DIR}/server.log"
PS_THREADS_FILE="${OUT_DIR}/ps_threads.txt"
TOP_THREADS_FILE="${OUT_DIR}/top_threads.txt"
PIDSTAT_U_FILE="${OUT_DIR}/pidstat_u.txt"
PIDSTAT_W_FILE="${OUT_DIR}/pidstat_w.txt"
VMSTAT_FILE="${OUT_DIR}/vmstat.txt"
STRACE_SUMMARY_FILE="${OUT_DIR}/strace_summary.txt"
PERF_REPORT_FILE="${OUT_DIR}/perf_report.txt"
PERF_DATA_FILE="${OUT_DIR}/perf.data"
PERF_RECORD_LOG_FILE="${OUT_DIR}/perf_record.log"
SUMMARY_TXT="${OUT_DIR}/summary.txt"
META_JSON="${OUT_DIR}/meta.json"

PERF_EVENT="${PERF_EVENT:-cpu-clock}"
PERF_FREQ="${PERF_FREQ:-99}"
PERF_CALL_GRAPH="${PERF_CALL_GRAPH:-fp}"

mkdir -p "${OUT_DIR}"

log() {
  echo "[profile_quick] $*"
}

have_cmd() {
  command -v "$1" >/dev/null 2>&1
}

wait_for_webgame_pid() {
  local retries="${1:-20}"
  local interval="${2:-1}"
  local pid=""
  for _ in $(seq 1 "${retries}"); do
    pid="$(pgrep -n WebGame || true)"
    if [[ -n "${pid}" ]]; then
      echo "${pid}"
      return 0
    fi
    sleep "${interval}"
  done
  return 1
}

snapshot_threads() {
  if have_cmd ps; then
    ps -L -p "${TARGET_PID}" -o pid,tid,pcpu,comm --sort=-pcpu > "${PS_THREADS_FILE}" || true
  fi
  if have_cmd top; then
    top -H -b -n 1 -p "${TARGET_PID}" > "${TOP_THREADS_FILE}" || true
  fi
}

COLLECTOR_PIDS=()
STRACE_PID=""
PERF_PID=""

start_collectors() {
  local mode="$1"
  local count_arg=()
  if [[ "${mode}" == "timed" ]]; then
    count_arg=("${DURATION_SEC}")
  fi

  if have_cmd pidstat; then
    pidstat -u -t -p "${TARGET_PID}" "${SAMPLE_INTERVAL_SEC}" "${count_arg[@]}" > "${PIDSTAT_U_FILE}" 2>&1 &
    COLLECTOR_PIDS+=("$!")
    pidstat -w -t -p "${TARGET_PID}" "${SAMPLE_INTERVAL_SEC}" "${count_arg[@]}" > "${PIDSTAT_W_FILE}" 2>&1 &
    COLLECTOR_PIDS+=("$!")
  fi

  if have_cmd vmstat; then
    vmstat "${SAMPLE_INTERVAL_SEC}" "${count_arg[@]}" > "${VMSTAT_FILE}" 2>&1 &
    COLLECTOR_PIDS+=("$!")
  fi

  if have_cmd strace; then
    strace -f -p "${TARGET_PID}" -c -qq -e trace=network,futex,epoll_wait,poll > "${STRACE_SUMMARY_FILE}" 2>&1 &
    STRACE_PID="$!"
  fi

  if have_cmd perf; then
    perf record \
      -e "${PERF_EVENT}" \
      -F "${PERF_FREQ}" \
      --call-graph "${PERF_CALL_GRAPH}" \
      -p "${TARGET_PID}" \
      -o "${PERF_DATA_FILE}" \
      >"${PERF_RECORD_LOG_FILE}" 2>&1 &
    PERF_PID="$!"
  fi
}

stop_collectors() {
  for pid in "${COLLECTOR_PIDS[@]:-}"; do
    if [[ -n "${pid}" ]] && ps -p "${pid}" >/dev/null 2>&1; then
      kill -TERM "${pid}" >/dev/null 2>&1 || true
      wait "${pid}" >/dev/null 2>&1 || true
    fi
  done

  if [[ -n "${STRACE_PID}" ]] && ps -p "${STRACE_PID}" >/dev/null 2>&1; then
    kill -INT "${STRACE_PID}" >/dev/null 2>&1 || true
    wait "${STRACE_PID}" >/dev/null 2>&1 || true
  fi

  if [[ -n "${PERF_PID}" ]] && ps -p "${PERF_PID}" >/dev/null 2>&1; then
    kill -INT "${PERF_PID}" >/dev/null 2>&1 || true
    wait "${PERF_PID}" >/dev/null 2>&1 || true
  fi

  if [[ -f "${PERF_DATA_FILE}" ]] && have_cmd perf; then
    if perf report --stdio -i "${PERF_DATA_FILE}" > "${PERF_REPORT_FILE}" 2>&1; then
      if grep -q "has no samples" "${PERF_REPORT_FILE}"; then
        {
          echo
          echo "hint: perf data has no samples. common causes:"
          echo "  1) profiling window had almost no CPU workload (e.g. RUN_MATRIX=0 and no external traffic)"
          echo "  2) event unsupported on host/kernel"
          echo "  3) perf permission limits (perf_event_paranoid)"
          echo ""
          echo "suggestion: run with load + timed mode, for example:"
          echo "  sudo PROFILE_MODE=timed DURATION_SEC=20 RUN_MATRIX=1 PERF_EVENT=cpu-clock ./scripts/profile_quick.sh"
          echo ""
          echo "perf header:"
        } >> "${PERF_REPORT_FILE}"
        perf report --header-only -i "${PERF_DATA_FILE}" >> "${PERF_REPORT_FILE}" 2>&1 || true
      fi
    else
      {
        echo "perf report failed (likely permission/perf_event_paranoid or invalid perf.data)"
        if [[ -f "${PERF_RECORD_LOG_FILE}" ]]; then
          echo
          echo "perf record log:"
          cat "${PERF_RECORD_LOG_FILE}"
        fi
      } > "${PERF_REPORT_FILE}"
    fi
  elif have_cmd perf; then
    {
      echo "perf record failed (likely permission/perf_event_paranoid or unsupported event)"
      if [[ -f "${PERF_RECORD_LOG_FILE}" ]]; then
        echo
        echo "perf record log:"
        cat "${PERF_RECORD_LOG_FILE}"
      fi
    } > "${PERF_REPORT_FILE}"
  fi
}

SERVER_PID=""
MATRIX_JOB_PID=""

cleanup() {
  stop_collectors || true
  if [[ -n "${MATRIX_JOB_PID}" ]] && ps -p "${MATRIX_JOB_PID}" >/dev/null 2>&1; then
    kill -INT "${MATRIX_JOB_PID}" >/dev/null 2>&1 || true
    wait "${MATRIX_JOB_PID}" >/dev/null 2>&1 || true
  fi
  if [[ -n "${SERVER_PID}" ]] && ps -p "${SERVER_PID}" >/dev/null 2>&1; then
    kill -INT "${SERVER_PID}" >/dev/null 2>&1 || true
    wait "${SERVER_PID}" >/dev/null 2>&1 || true
  fi
}
trap cleanup EXIT

if [[ "${PROFILE_MODE}" == "interactive" && "${START_SERVER}" == "1" ]]; then
  if [[ ! -x "${WEBGAME_BIN}" ]]; then
    log "cannot start server: binary not found ${WEBGAME_BIN}"
    exit 3
  fi
  log "starting server binary in background"
  (
    cd "${ROOT_DIR}"
    WEBGAME_HOST="${WEBGAME_HOST:-0.0.0.0}" WEBGAME_PORT="${WEBGAME_PORT:-18888}" "${WEBGAME_BIN}"
  ) >"${SERVER_LOG}" 2>&1 &
  SERVER_PID=$!
  sleep "${WARMUP_SEC}"
fi

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
  TARGET_PID="$(wait_for_webgame_pid 30 1 || true)"
fi

if [[ -z "${TARGET_PID}" ]]; then
  log "cannot find WebGame pid, abort"
  if [[ -n "${MATRIX_JOB_PID}" ]]; then
    wait "${MATRIX_JOB_PID}" || true
  fi
  exit 2
fi

log "target pid=${TARGET_PID}"
snapshot_threads

START_TS="$(date +%s)"
start_collectors "${PROFILE_MODE}"

if [[ "${PROFILE_MODE}" == "interactive" ]]; then
  log "interactive mode: type '${STOP_COMMAND}' and press Enter to finish profiling"
  while true; do
    printf "> "
    if ! IFS= read -r cmd; then
      break
    fi
    if [[ "${cmd}" == "${STOP_COMMAND}" ]]; then
      break
    fi
  done
else
  sleep "${DURATION_SEC}"
fi

stop_collectors
END_TS="$(date +%s)"
ACTUAL_DURATION="$((END_TS - START_TS))"
if [[ "${ACTUAL_DURATION}" -le 0 ]]; then
  ACTUAL_DURATION=1
fi

if [[ -n "${SERVER_PID}" ]] && ps -p "${SERVER_PID}" >/dev/null 2>&1; then
  log "stopping server"
  kill -INT "${SERVER_PID}" >/dev/null 2>&1 || true
  wait "${SERVER_PID}" >/dev/null 2>&1 || true
fi

MATRIX_EXIT=0
if [[ -n "${MATRIX_JOB_PID}" ]]; then
  wait "${MATRIX_JOB_PID}" || MATRIX_EXIT=$?
fi

python3 - <<'PY' \
"${OUT_DIR}" "${TARGET_PID}" "${ACTUAL_DURATION}" "${RUN_MATRIX}" "${MATRIX_EXIT}" "${PROFILE_MODE}" \
"${MATRIX_LOG}" "${SERVER_LOG}" "${PS_THREADS_FILE}" "${PIDSTAT_W_FILE}" "${VMSTAT_FILE}" "${STRACE_SUMMARY_FILE}" "${PERF_REPORT_FILE}" "${PERF_RECORD_LOG_FILE}" "${SUMMARY_TXT}" "${META_JSON}"
import json, os, re, sys
(
    out_dir, target_pid, duration_sec, run_matrix, matrix_exit, profile_mode,
    matrix_log, server_log, ps_threads_file, pidstat_w_file, vmstat_file,
  strace_summary_file, perf_report_file, perf_record_log_file, summary_txt, meta_json
) = sys.argv[1:]

result = {
    "target_pid": int(target_pid),
    "duration_sec": int(duration_sec),
    "profile_mode": profile_mode,
    "run_matrix": run_matrix == "1",
    "matrix_exit": int(matrix_exit),
    "artifacts": {
        "out_dir": out_dir,
        "matrix_log": matrix_log if os.path.exists(matrix_log) else None,
        "server_log": server_log if os.path.exists(server_log) else None,
        "ps_threads": ps_threads_file if os.path.exists(ps_threads_file) else None,
        "pidstat_w": pidstat_w_file if os.path.exists(pidstat_w_file) else None,
        "vmstat": vmstat_file if os.path.exists(vmstat_file) else None,
        "strace_summary": strace_summary_file if os.path.exists(strace_summary_file) else None,
        "perf_report": perf_report_file if os.path.exists(perf_report_file) else None,
        "perf_record_log": perf_record_log_file if os.path.exists(perf_record_log_file) else None,
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

perf_no_samples = False
if os.path.exists(perf_report_file):
  with open(perf_report_file, "r", encoding="utf-8", errors="ignore") as f:
    report_txt = f.read()
  if "has no samples" in report_txt:
    perf_no_samples = True

if perf_no_samples:
  if run_matrix == "0" and (max_thread_cpu is None or max_thread_cpu < 1.0):
    diagnosis.append("perf 无样本：采样期间几乎无负载（当前 run_matrix=0 且线程 CPU 很低）")
  else:
    diagnosis.append("perf 无样本：建议检查 PERF_EVENT / 权限(perf_event_paranoid) / 宿主机 PMU 支持")

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
lines.append(f"profile_mode={profile_mode}")
lines.append(f"run_matrix={run_matrix}")
lines.append(f"matrix_exit={matrix_exit}")
lines.append("")
lines.append("collected_metrics")
lines.append("  - thread cpu snapshot (ps/top)")
lines.append("  - process/thread cpu usage (pidstat -u)")
lines.append("  - context switches (pidstat -w)")
lines.append("  - system run queue (vmstat)")
lines.append("  - syscall share (strace -c, if permitted)")
lines.append("  - cpu hotspots (perf, if permitted)")
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
log "mode: ${PROFILE_MODE}"
log "collected: thread cpu, pidstat(u/w), vmstat, strace, perf"
log "summary: ${SUMMARY_TXT}"
log "meta: ${META_JSON}"
