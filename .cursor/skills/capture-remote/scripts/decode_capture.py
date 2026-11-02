#!/usr/bin/env python3
"""Decode one Daikin ARC486A56 raw capture from an ESPHome USB log or a .raw.txt file."""

import argparse
import re
import sys
from pathlib import Path


def parse_numbers(text):
    return [int(item) for item in re.findall(r"-?\d+", text)]


def parse_hex_bytes(text):
    return [int(item, 16) for item in text.split()]


def bursts_from_log(text):
    """Return (timestamp, pulses) for each Received Raw block."""
    bursts = []
    current_time = None
    current_parts = []

    def flush():
        if current_time is None:
            return
        pulses = []
        for part in current_parts:
            pulses.extend(parse_numbers(part))
        if pulses:
            bursts.append((current_time, pulses))

    for line in text.splitlines():
        if "Received Raw:" in line:
            flush()
            stamp = re.search(r"\[(\d{2}:\d{2}:\d{2}\.\d+)\]", line)
            current_time = stamp.group(1) if stamp else ""
            current_parts = [line.split("Received Raw:", 1)[1]]
            continue
        if current_time is not None and "remote.raw" in line:
            current_parts.append(line.split("]:", 1)[-1])
            continue
        if current_time is not None and line.strip():
            flush()
            current_time = None
            current_parts = []
    flush()
    return bursts


def decode(pulses):
    """Return a list of byte lists. A 6-bit preamble is skipped."""
    sections = []
    bits = []

    def flush():
        nonlocal bits
        if not bits:
            return
        if len(bits) >= 8 and len(bits) % 8 == 0:
            data = []
            for start in range(0, len(bits), 8):
                value = 0
                for shift in range(8):
                    value |= bits[start + shift] << shift
                data.append(value)
            sections.append(data)
        bits = []

    index = 0
    while index < len(pulses):
        mark = pulses[index]
        if mark > 2000:
            flush()
            index += 2
            continue
        if mark > 0 and index + 1 < len(pulses) and pulses[index + 1] < 0:
            space = pulses[index + 1]
            if space < -1800:
                flush()
            else:
                bits.append(1 if space < -800 else 0)
            index += 2
            continue
        index += 1
    flush()
    return sections


def hex_bytes(data):
    return " ".join(f"{value:02X}" for value in data)


def checksum_ok(data):
    return bool(data) and (sum(data[:-1]) & 0xFF) == data[-1]


def write_raw(path, pulses):
    lines = []
    for start in range(0, len(pulses), 16):
        lines.append(", ".join(str(value) for value in pulses[start : start + 16]))
    Path(path).write_text("\n".join(lines) + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--log", help="ESPHome USB log, including a Cursor terminal file")
    source.add_argument("--raw", help="Stored pulse file")
    parser.add_argument("--after", default="", help="Ignore log bursts at or before HH:MM:SS.mmm")
    parser.add_argument("--index", type=int, default=-1, help="Burst index after filtering. Default: last")
    parser.add_argument("--compare", default="", help="Previous frame bytes, hex, for a byte diff")
    parser.add_argument("--write-raw", help="Write the selected pulse train, 16 numbers per line")
    args = parser.parse_args()

    timestamp = ""
    if args.log:
        bursts = bursts_from_log(Path(args.log).read_text(errors="replace"))
        if args.after:
            bursts = [item for item in bursts if item[0] > args.after]
        if not bursts:
            print("No Received Raw block found", file=sys.stderr)
            return 1
        timestamp, pulses = bursts[args.index]
    else:
        pulses = parse_numbers(Path(args.raw).read_text())

    frames = decode(pulses)
    print(f"timestamp: {timestamp or '-'}")
    print(f"pulses: {len(pulses)}")
    if len(frames) != 3:
        print(f"frames: {len(frames)} (expected 3)")
    for number, frame in enumerate(frames, start=1):
        print(
            f"frame {number}: {hex_bytes(frame)} checksum={'ok' if checksum_ok(frame) else 'BAD'}"
        )

    if args.compare:
        previous = parse_hex_bytes(args.compare)
        print("diff:")
        width = max(len(previous), len(frames[-1]) if frames else 0)
        found = False
        for index in range(width):
            old = previous[index] if index < len(previous) else None
            new = frames[-1][index] if frames and index < len(frames[-1]) else None
            if old != new:
                found = True
                old_text = "--" if old is None else f"{old:02X}"
                new_text = "--" if new is None else f"{new:02X}"
                print(f"  byte {index}: {old_text} -> {new_text}")
        if not found:
            print("  no byte changes")

    if args.write_raw:
        write_raw(args.write_raw, pulses)
        print(f"wrote: {args.write_raw}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
