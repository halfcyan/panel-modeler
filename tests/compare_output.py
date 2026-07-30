#!/usr/bin/env python3
"""Run the panel-modeler CLI on an example input and compare its output
byte-for-byte with an expected CSV. Used by `meson test`."""

import pathlib
import subprocess
import sys
import tempfile


def main() -> int:
    if len(sys.argv) != 4:
        print("usage: compare_output.py <exe> <input.csv> <expected.csv>", file=sys.stderr)
        return 2
    exe, input_csv, expected_csv = sys.argv[1:4]

    with tempfile.TemporaryDirectory() as tmp:
        output_csv = pathlib.Path(tmp) / "actual.csv"
        result = subprocess.run([exe, input_csv, str(output_csv)], capture_output=True, text=True)
        if result.returncode != 0:
            sys.stderr.write(result.stdout)
            sys.stderr.write(result.stderr)
            return 1
        actual = output_csv.read_bytes()

    expected = pathlib.Path(expected_csv).read_bytes()
    if actual == expected:
        return 0

    actual_lines = actual.decode().splitlines()
    expected_lines = expected.decode().splitlines()
    for i, pair in enumerate(zip(actual_lines, expected_lines)):
        if pair[0] != pair[1]:
            print(f"first difference on line {i + 1}:", file=sys.stderr)
            print(f"  actual:   {pair[0]!r}", file=sys.stderr)
            print(f"  expected: {pair[1]!r}", file=sys.stderr)
            break
    print(f"line counts: actual {len(actual_lines)}, expected {len(expected_lines)}", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
