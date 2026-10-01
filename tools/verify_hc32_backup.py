# SPDX-License-Identifier: GPL-3.0-or-later
"""Verify/assemble a 64 KiB HC32 backup from Intel HEX exports; no board access."""
import argparse
import hashlib
from pathlib import Path

def assemble(paths):
    memory = {}
    for path in paths:
        base, eof = 0, False
        for number, line in enumerate(path.read_text(encoding="ascii").splitlines(), 1):
            if not line.strip():
                continue
            if eof or not line.startswith(":"):
                raise ValueError(f"{path}:{number}: invalid record or data after EOF")
            record = bytes.fromhex(line[1:])
            if len(record) < 5 or len(record) != record[0] + 5 or sum(record) & 255:
                raise ValueError(f"{path}:{number}: invalid length/checksum")
            length, address, kind = record[0], int.from_bytes(record[1:3], "big"), record[3]
            data = record[4:-1]
            if kind == 0:
                for offset, byte in enumerate(data):
                    absolute = base + address + offset
                    if not 0 <= absolute < 65536 or absolute in memory:
                        raise ValueError(f"{path}:{number}: out-of-range or overlapping address")
                    memory[absolute] = byte
            elif kind == 1 and length == 0 and address == 0:
                eof = True
            elif kind in (2, 4) and length == 2 and address == 0:
                base = int.from_bytes(data, "big") << (4 if kind == 2 else 16)
            elif kind in (3, 5) and length == 4 and address == 0:
                pass  # Entry-point metadata; not flash contents.
            else:
                raise ValueError(f"{path}:{number}: unsupported or malformed record")
        if not eof:
            raise ValueError(f"{path}: missing EOF")
    if len(memory) != 65536:
        raise ValueError(f"Incomplete flash: {len(memory)} / 65536 bytes")
    return bytes(memory[i] for i in range(65536))

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", type=Path)
    parser.add_argument("--output", type=Path, help="Optional new binary; never overwrites")
    parser.add_argument("--expected-sha256", help="Optional comparison with a known backup")
    args = parser.parse_args()
    try:
        data = assemble(args.inputs)
        digest = hashlib.sha256(data).hexdigest().upper()
        if args.expected_sha256 and digest != args.expected_sha256.upper():
            raise ValueError("SHA-256 does not match expected backup")
        if args.output:
            with args.output.open("xb") as output:
                output.write(data)
        print(f"PASS: 65536 bytes, no gaps/overlaps; SHA-256 {digest}")
    except (ValueError, OSError) as error:
        parser.exit(1, f"ERROR: {error}\n")

if __name__ == "__main__":
    main()
