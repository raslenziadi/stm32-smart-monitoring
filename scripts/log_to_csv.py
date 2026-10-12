#!/usr/bin/env python3
"""Convert native_sim monitoring logs to the visualization CSV format."""

import argparse
import csv
import re
import sys


MEASUREMENT_PATTERN = re.compile(
    r"cycle=(?P<cycle>\d+) timestamp_ms=(?P<timestamp>\d+) "
    r"temp=(?P<temperature>-?\d+\.\d+)C "
    r"humidity=(?P<humidity>-?\d+)% light=(?P<light>-?\d+) lux "
    r"valid=\((?P<temperature_valid>[01]),(?P<humidity_valid>[01]),"
    r"(?P<light_valid>[01])\)"
)
MONITOR_CYCLE_PATTERN = re.compile(r"Surveillance du cycle\s+(?P<cycle>\d+)")
SYSTEM_STATE_PATTERN = re.compile(r"system_state=(?P<state>[A-Z_]+)\b")
CSV_COLUMNS = (
    "timestamp",
    "cycle",
    "temperature",
    "humidity",
    "light",
    "system_state",
    "temperature_valid",
    "humidity_valid",
    "light_valid",
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert native_sim monitoring logs to CSV."
    )
    parser.add_argument("--input", required=True, help="Log file, or '-' for stdin.")
    parser.add_argument("--output", required=True, help="CSV output file.")
    return parser.parse_args()


def convert(log_lines: list[str]) -> list[dict[str, str]]:
    measurements: dict[int, dict[str, str]] = {}
    states: dict[int, str] = {}
    current_monitor_cycle: int | None = None

    for line in log_lines:
        monitor_match = MONITOR_CYCLE_PATTERN.search(line)
        if monitor_match:
            current_monitor_cycle = int(monitor_match.group("cycle"))

        state_match = SYSTEM_STATE_PATTERN.search(line)
        if state_match and current_monitor_cycle is not None:
            states[current_monitor_cycle] = state_match.group("state")

        measurement_match = MEASUREMENT_PATTERN.search(line)
        if measurement_match:
            values = measurement_match.groupdict()
            cycle = int(values.pop("cycle"))
            measurements[cycle] = {
                "cycle": str(cycle),
                "timestamp": values.pop("timestamp"),
                **values,
            }

    rows = []
    for cycle in sorted(measurements):
        measurement = measurements[cycle]
        state = states.get(cycle)
        if state is None:
            validity_flags = (
                measurement["temperature_valid"],
                measurement["humidity_valid"],
                measurement["light_valid"],
            )
            state = "INVALID" if "0" in validity_flags else "NO_STATE"
        measurement["system_state"] = state
        rows.append(measurement)
    return rows


def main() -> int:
    args = parse_args()
    try:
        if args.input == "-":
            rows = convert(sys.stdin)
        else:
            with open(args.input, encoding="utf-8") as log_file:
                rows = convert(log_file)

        if not rows:
            raise ValueError("No monitoring measurements found in the input.")

        with open(args.output, "w", newline="", encoding="utf-8") as csv_file:
            writer = csv.DictWriter(csv_file, fieldnames=CSV_COLUMNS)
            writer.writeheader()
            writer.writerows(rows)
    except (OSError, ValueError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1

    print(f"CSV saved to {args.output} ({len(rows)} measurements)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())