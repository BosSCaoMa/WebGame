#!/usr/bin/env python3
import argparse
import glob
import json
import os
from typing import Any


def safe_float(value: Any, default: float = 0.0) -> float:
    try:
        return float(value)
    except (TypeError, ValueError):
        return default


def safe_int(value: Any, default: int = 0) -> int:
    try:
        return int(value)
    except (TypeError, ValueError):
        return default


def load_rows(base_dir: str, flow_mode: str) -> list[dict[str, Any]]:
    files = sorted(glob.glob(os.path.join(base_dir, "battle_flow_bench_*", "summary.json")))
    rows: list[dict[str, Any]] = []
    for path in files:
        try:
            with open(path, "r", encoding="utf-8") as file:
                data = json.load(file)
        except (OSError, json.JSONDecodeError):
            continue

        mode = str(data.get("flow_mode", ""))
        if flow_mode and mode != flow_mode:
            continue

        throughput = data.get("throughput", {}) if isinstance(data, dict) else {}
        latency = data.get("flow_latency_ms", {}) if isinstance(data, dict) else {}

        rows.append(
            {
                "timestamp": str(data.get("timestamp", "")),
                "flow_mode": mode,
                "flow_users": safe_int(data.get("flow_users", 0)),
                "concurrency": safe_int(data.get("concurrency", 0)),
                "success_rate_percent": safe_float(throughput.get("success_rate_percent", 0)),
                "flow_rps": safe_float(throughput.get("flow_rps", 0)),
                "failed_flows": safe_int(throughput.get("failed_flows", 0)),
                "p95": safe_float(latency.get("p95", 0)),
                "avg": safe_float(latency.get("avg", 0)),
                "summary_json": path,
                "result_dir": os.path.dirname(path),
            }
        )

    rows.sort(key=lambda row: (row["flow_users"], row["concurrency"], row["timestamp"]))
    return rows


def best_row(rows: list[dict[str, Any]], predicate) -> dict[str, Any] | None:
    candidates = [row for row in rows if predicate(row)]
    if not candidates:
        return None
    return max(candidates, key=lambda row: row["flow_rps"])


def format_row(row: dict[str, Any]) -> str:
    return (
        f"users={row['flow_users']}, conc={row['concurrency']}, mode={row['flow_mode']}, "
        f"success={row['success_rate_percent']:.2f}%, rps={row['flow_rps']:.2f}, "
        f"p95={row['p95']:.0f}ms, avg={row['avg']:.1f}ms, failed={row['failed_flows']}, "
        f"dir={row['result_dir']}"
    )


def write_text_report(
    output_txt: str,
    rows: list[dict[str, Any]],
    peak: dict[str, Any] | None,
    stable: dict[str, Any] | None,
    lowlat: dict[str, Any] | None,
    flow_mode: str,
    stable_threshold: float,
    lowlat_threshold: float,
) -> None:
    lines: list[str] = []
    lines.append("battle_flow aggregate summary")
    lines.append(f"flow_mode={flow_mode or 'all'}")
    lines.append(f"stable_success_threshold={stable_threshold}")
    lines.append(f"lowlat_p95_threshold_ms={lowlat_threshold}")
    lines.append("")

    lines.append(
        "users\tconc\tsuccess_rate_percent\tflow_rps\tp95_ms\tavg_ms\tfailed_flows\tresult_dir"
    )
    for row in rows:
        lines.append(
            f"{row['flow_users']}\t{row['concurrency']}\t{row['success_rate_percent']:.2f}\t"
            f"{row['flow_rps']:.2f}\t{row['p95']:.0f}\t{row['avg']:.1f}\t{row['failed_flows']}\t"
            f"{row['result_dir']}"
        )

    lines.append("")
    lines.append("peak")
    lines.append(format_row(peak) if peak else "none")
    lines.append("")
    lines.append("stable")
    lines.append(format_row(stable) if stable else "none")
    lines.append("")
    lines.append("lowlat")
    lines.append(format_row(lowlat) if lowlat else "none")

    os.makedirs(os.path.dirname(output_txt), exist_ok=True)
    with open(output_txt, "w", encoding="utf-8") as file:
        file.write("\n".join(lines) + "\n")


def write_json_report(
    output_json: str,
    rows: list[dict[str, Any]],
    peak: dict[str, Any] | None,
    stable: dict[str, Any] | None,
    lowlat: dict[str, Any] | None,
    flow_mode: str,
    stable_threshold: float,
    lowlat_threshold: float,
) -> None:
    payload = {
        "flow_mode": flow_mode or "all",
        "stable_success_threshold": stable_threshold,
        "lowlat_p95_threshold_ms": lowlat_threshold,
        "total_runs": len(rows),
        "rows": rows,
        "peak": peak,
        "stable": stable,
        "lowlat": lowlat,
    }
    os.makedirs(os.path.dirname(output_json), exist_ok=True)
    with open(output_json, "w", encoding="utf-8") as file:
        json.dump(payload, file, ensure_ascii=False, indent=2)


def main() -> int:
    parser = argparse.ArgumentParser(description="Aggregate battle flow benchmark summaries.")
    parser.add_argument(
        "--base-dir",
        default="build/benchmarks/battle_flow",
        help="Base directory containing battle_flow_bench_* result folders.",
    )
    parser.add_argument(
        "--flow-mode",
        default="auto",
        help="Filter by flow mode. Use empty string to include all modes.",
    )
    parser.add_argument(
        "--stable-success-threshold",
        type=float,
        default=99.9,
        help="Minimum success rate percentage for stable point.",
    )
    parser.add_argument(
        "--lowlat-p95-threshold",
        type=float,
        default=10000,
        help="Maximum p95 latency in ms for low-lat point.",
    )
    parser.add_argument(
        "--output-txt",
        default=None,
        help="Output text report path. Defaults to <base-dir>/aggregate_summary.txt",
    )
    parser.add_argument(
        "--output-json",
        default=None,
        help="Output json report path. Defaults to <base-dir>/aggregate_summary.json",
    )
    args = parser.parse_args()

    base_dir = os.path.abspath(args.base_dir)
    flow_mode = args.flow_mode
    rows = load_rows(base_dir, flow_mode)

    output_txt = args.output_txt or os.path.join(base_dir, "aggregate_summary.txt")
    output_json = args.output_json or os.path.join(base_dir, "aggregate_summary.json")

    peak = best_row(rows, lambda _: True)
    stable = best_row(rows, lambda row: row["success_rate_percent"] >= args.stable_success_threshold)
    lowlat = best_row(
        rows,
        lambda row: row["success_rate_percent"] >= args.stable_success_threshold
        and row["p95"] <= args.lowlat_p95_threshold,
    )

    write_text_report(
        output_txt,
        rows,
        peak,
        stable,
        lowlat,
        flow_mode,
        args.stable_success_threshold,
        args.lowlat_p95_threshold,
    )
    write_json_report(
        output_json,
        rows,
        peak,
        stable,
        lowlat,
        flow_mode,
        args.stable_success_threshold,
        args.lowlat_p95_threshold,
    )

    print(f"[summarize_battle_flow] total_runs={len(rows)}")
    print(f"[summarize_battle_flow] text={output_txt}")
    print(f"[summarize_battle_flow] json={output_json}")
    if peak:
        print(f"[summarize_battle_flow] peak={format_row(peak)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
