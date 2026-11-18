"""Convert the pinned private Fleet bank to a bank with palette fingerprints."""
import argparse
import hashlib
import json
import struct
from pathlib import Path

V1_SHA256 = "08ae3f6f29bd31994d81c323cc8251429c32e25eb6a2613a880cd438145a495a"
COUNTS = (139, 2)
KEYS = {0xB200, 0xB272}
ENTRY = struct.Struct("<IIHhhHHII32s")


def digest(data):
    return hashlib.sha256(data).hexdigest()


def convert(original):
    if digest(original) != V1_SHA256 or original[:8] != b"DWFLEET1":
        raise ValueError("Fleet V1 input identity mismatch")
    counts = struct.unpack_from("<II", original, 8)
    if counts != COUNTS:
        raise ValueError("Unexpected V1 record counts")

    position = 16
    palette_records = bytearray()
    ramps = {}
    for _ in range(counts[1]):
        key, = struct.unpack_from("<I", original, position)
        ramp = original[position + 4:position + 516]
        if key not in KEYS or key in ramps or len(ramp) != 512:
            raise ValueError("Malformed V1 palette record")
        ramps[key] = ramp
        palette_records += struct.pack("<I", key) + hashlib.sha256(ramp).digest()
        position += 516

    # Keep every identity field and every replacement channel byte from the
    # accepted trial. Check the record boundaries before copying the section.
    entries = original[position:]
    position = 0
    seen = set()
    for _ in range(counts[0]):
        key, palette, flags, _, _, width, height, pw, ph, _ = ENTRY.unpack_from(entries, position)
        if ((key, palette) in seen or palette not in KEYS or flags not in (1, 3)
                or not (0 < width <= 800 and 0 < height <= 600)
                or (pw, ph) != (width * 4, height * 4)):
            raise ValueError("Malformed V1 entry")
        seen.add((key, palette))
        position += ENTRY.size + pw * ph * 4
        if position > len(entries):
            raise ValueError("Truncated V1 entry")
    if position != len(entries):
        raise ValueError("Bytes remain after the V1 entries")

    output = b"DWFLEET2" + original[8:16] + palette_records + entries
    if len(output) != len(original) - 2 * (512 - 32) or any(r in output for r in ramps.values()):
        raise ValueError("Unexpected V2 payload")
    receipt = {
        "format": "DWFLEET2",
        "source_sha256": V1_SHA256,
        "output_sha256": digest(output),
        "source_bytes": len(original),
        "output_bytes": len(output),
        "entries": counts[0],
        "entry_section_sha256": digest(entries),
        "entry_section_byte_identical": True,
        "palette_sha256": {f"{key:04X}": digest(ramp) for key, ramp in ramps.items()},
        "embedded_literal_palette_matches": 0,
    }
    return output, receipt


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("private_v1", type=Path)
    parser.add_argument("output_v2", type=Path)
    parser.add_argument("--receipt", type=Path, required=True)
    args = parser.parse_args()
    output, receipt = convert(args.private_v1.read_bytes())
    if args.output_v2.exists() or args.receipt.exists():
        parser.error("Use new output and receipt paths")
    with args.output_v2.open("xb") as file:
        file.write(output)
    with args.receipt.open("x", encoding="utf-8") as file:
        json.dump(receipt, file, indent=2, sort_keys=True)
        file.write("\n")


if __name__ == "__main__":
    main()
