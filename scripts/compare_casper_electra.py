#!/usr/bin/env python3
"""Compile and run the Casper YKR vs ELECTRA_AC host comparison.

The C++ harness encodes states with the firmware frame builder and with an
Electra bitfield, then diffs bytes and the sendElectraAC pulse string.
"""

import subprocess
import sys
from pathlib import Path


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    source = root / "scripts" / "electra_ac_host.cc"
    include = root / "firmware" / "esphome" / "components" / "casper_ykr"
    binary = Path("/tmp/electra_ac_host")
    compile_cmd = [
        "g++",
        "-std=c++17",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-I",
        str(include),
        str(source),
        "-o",
        str(binary),
    ]
    print(" ".join(compile_cmd), flush=True)
    subprocess.check_call(compile_cmd)
    return subprocess.call([str(binary)])


if __name__ == "__main__":
    sys.exit(main())
