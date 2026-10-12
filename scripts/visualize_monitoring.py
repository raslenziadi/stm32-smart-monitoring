#!/usr/bin/env python3
"""Visualize a CSV export of monitoring data."""

import argparse
import csv
import os
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

REQUIRED_COLUMNS = {
    "timestamp",
    "cycle",
    "temperature",
    "humidity",
    "light",
    "system_state",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Plot temperature, humidity, light and system state trends from a CSV file."
    )
    parser.add_argument("--input", required=True, help="CSV file to analyze.")
    parser.add_argument(
        "--output",
        default="monitoring_plot.png",
        help="Output image file to generate.",
    )
    return parser.parse_args()


def load_rows(path: str):
    if not os.path.exists(path):
        raise FileNotFoundError(f"Input file not found: {path}")

    with open(path, newline="", encoding="utf-8") as csv_file:
        reader = csv.DictReader(csv_file)
        if reader.fieldnames is None:
            raise ValueError("CSV file is empty or missing a header row.")

        missing = REQUIRED_COLUMNS - set(reader.fieldnames)
        if missing:
            raise ValueError(f"Missing required columns: {sorted(missing)}")

        rows = list(reader)

    if not rows:
        raise ValueError("CSV file does not contain any rows.")

    return rows


def plot(rows, output_path: str) -> None:
    cycles = [int(row["cycle"]) for row in rows]
    temperatures = [float(row["temperature"]) for row in rows]
    humidities = [float(row["humidity"]) for row in rows]
    lights = [float(row["light"]) for row in rows]
    states = [row["system_state"] for row in rows]

    fig, axes = plt.subplots(2, 2, figsize=(12, 8))
    axes[0, 0].plot(cycles, temperatures, marker="o")
    axes[0, 0].set_title("Temperature")
    axes[0, 0].set_ylabel("°C")

    axes[0, 1].plot(cycles, humidities, marker="o", color="tab:green")
    axes[0, 1].set_title("Humidity")
    axes[0, 1].set_ylabel("%")

    axes[1, 0].plot(cycles, lights, marker="o", color="tab:orange")
    axes[1, 0].set_title("Light")
    axes[1, 0].set_ylabel("lux")

    unique_states = sorted(set(states))
    counts = {state: states.count(state) for state in unique_states}
    axes[1, 1].bar(counts.keys(), counts.values(), color="tab:purple")
    axes[1, 1].set_title("System states")
    axes[1, 1].set_ylabel("Count")

    fig.tight_layout()
    fig.savefig(output_path, dpi=180)
    print(f"Chart saved to {output_path}")
    print("State counts:")
    for state, count in counts.items():
        print(f"  {state}: {count}")


def main() -> int:
    args = parse_args()

    try:
        rows = load_rows(args.input)
        plot(rows, args.output)
    except (FileNotFoundError, ValueError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
