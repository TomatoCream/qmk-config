#!/usr/bin/env python3
"""Apply a VIA backup JSON to the running keyboard's dynamic keymap.

The input is the same `{"name": ..., "macros": [...], "layers": [[...]]}`
format that `via-dump.py` writes and VIA's "Save + Load" tab uses. Only
positions whose value differs are written, so the board keeps anything the
JSON does not mention explicitly only through its keycodes, not through its
EEPROM layout. Nothing is flashed.

    ./via-apply.py keymap.json --vid 0x3434 --pid 0x03A1 --rows 6 --cols 16
    ./via-apply.py keymap.json --layer 0 --check    # report, exit 1 on drift

Layer indices are VIA layers (`--layer` may repeat); positions are row-major in
the `rows x cols` matrix, exactly like `via-dump.py`.
"""

from __future__ import annotations

import argparse
import importlib.util
import json
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
_spec = importlib.util.spec_from_file_location("via_dump", HERE / "via-dump.py")
via_dump = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(via_dump)

# QMK modifier bits as encoded in the high byte of a QK_MODS keycode.
MOD_BITS = {
    "LCTL": 0x01,
    "LSFT": 0x02,
    "LALT": 0x04,
    "LGUI": 0x08,
    "RCTL": 0x11,
    "RSFT": 0x12,
    "RALT": 0x14,
    "RGUI": 0x18,
}
QK_RANGES = {"TO": 0x5200, "MO": 0x5220, "DF": 0x5240, "TG": 0x5260, "OSL": 0x5280, "TT": 0x52C0}
CALL = re.compile(r"(\w+)\((.*)\)")


def name_values(codes: dict[int, dict]) -> dict[str, int]:
    names: dict[str, int] = {}
    for value, entry in codes.items():
        for name in [entry["key"], *entry.get("aliases", [])]:
            if name and set(name) != {"_"}:
                names.setdefault(name, value)
    return names


def resolve(expr: str, names: dict[str, int], depth: int = 0) -> int:
    expr = expr.strip()
    if depth > 8:
        raise SystemExit(f"Keycode expression nested too deeply: {expr!r}")
    if expr in names:
        return names[expr]
    if set(expr) == {"_"}:
        return names["KC_TRNS"]
    try:
        return int(expr, 0)
    except ValueError:
        pass
    match = CALL.fullmatch(expr)
    if not match:
        raise SystemExit(f"Unknown keycode expression: {expr!r}")
    func, arg = match.groups()
    if func in QK_RANGES:
        return QK_RANGES[func] | int(arg, 0)
    if func in MOD_BITS:
        return (MOD_BITS[func] << 8) | resolve(arg, names, depth + 1)
    raise SystemExit(f"Unknown keycode expression: {expr!r}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("backup", help="VIA backup JSON to load")
    parser.add_argument("--layer", type=int, action="append", help="only apply this VIA layer (repeatable; default: every layer in the file)")
    parser.add_argument("--check", action="store_true", help="report differing positions without writing; exit 1 when there are any")
    parser.add_argument("--qmk-home", default=str(HERE / "qmk_firmware"))
    parser.add_argument("--rows", type=int, default=12)
    parser.add_argument("--cols", type=int, default=6)
    parser.add_argument("--vid", type=lambda v: int(v, 0), default=0x594B)
    parser.add_argument("--pid", type=lambda v: int(v, 0), default=0x0D52)
    parser.add_argument("--device", help="raw HID path (default: first matching usage page 0xFF60)")
    args = parser.parse_args()

    backup = json.loads(Path(args.backup).read_text())
    codes = via_dump.load_keycodes(Path(args.qmk_home))
    names = name_values(codes)
    device = args.device or via_dump.find_raw_hid(args.vid, args.pid)
    via = via_dump.Via(device)

    available = via.layer_count()
    deltas: list[tuple[int, int, int, int, int]] = []
    for layer, keys in enumerate(backup.get("layers", [])):
        if args.layer and layer not in args.layer:
            continue
        if layer >= available:
            print(f"Skipping layer {layer}: board has {available} layers", file=sys.stderr)
            continue
        if len(keys) < args.rows * args.cols:
            raise SystemExit(f"Layer {layer} has {len(keys)} keycodes, expected {args.rows * args.cols}")
        for index in range(args.rows * args.cols):
            wanted = resolve(keys[index], names)
            row, col = divmod(index, args.cols)
            current = via.keycode(layer, row, col)
            if current != wanted:
                deltas.append((layer, row, col, current, wanted))

    for layer, row, col, current, wanted in deltas:
        print(
            f"layer {layer} [{row},{col}] "
            f"{via_dump.render_keycode(current, codes)} -> {via_dump.render_keycode(wanted, codes)}"
        )

    if args.check:
        print(f"{len(deltas)} differing position(s) on {device}", file=sys.stderr)
        return 1 if deltas else 0

    for layer, row, col, _current, wanted in deltas:
        via.transfer(bytes([0x05, layer, row, col, wanted >> 8, wanted & 0xFF]))

    for layer, row, col, current, wanted in deltas:
        written = via.keycode(layer, row, col)
        if written != wanted:
            raise SystemExit(
                f"Write did not stick at layer {layer} [{row},{col}]: "
                f"expected {via_dump.render_keycode(wanted, codes)}, "
                f"read {via_dump.render_keycode(written, codes)}"
            )

    print(f"Wrote {len(deltas)} position(s) to {device}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
