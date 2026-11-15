#!/usr/bin/env python3
"""Dump the running keyboard's VIA dynamic keymap to a VIA backup JSON.

The JSON layout matches what VIA's "Save + Load" tab writes, so
`qmk via2json` can map it back to the LAYOUT macro order:

    {"name": ..., "macros": [...], "layers": [[keycode, ...], ...]}

Layers are read over the raw HID (usage page 0xFF60) interface with the VIA
protocol; keycodes are turned back into C expressions (KC_A, MO(1),
LSFT(KC_9), ...) using QMK's keycode data. No VIA GUI or saved file needed.

With --reset the direction is reversed: the board's dynamic keymap is reseeded
from keymap.c (VIA's id_dynamic_keymap_reset), which is what a first boot or a
Bootmagic EEPROM reset would do — without leaving the running firmware.
"""

from __future__ import annotations

import argparse
import fcntl
import glob
import json
import os
import select
import struct
import sys
from pathlib import Path

RAW_HID_USAGE = b"\x06\x60\xff"  # Usage Page (Vendor 0xFF60)
HIDIOCGRDESCSIZE = 0x80044801
HIDIOCGRDESC = 0x90044802

# QMK modifier bits as encoded in the high byte of a QK_MODS keycode.
MODS = (
    (0x11, "RCTL"),
    (0x12, "RSFT"),
    (0x14, "RALT"),
    (0x18, "RGUI"),
    (0x01, "LCTL"),
    (0x02, "LSFT"),
    (0x04, "LALT"),
    (0x08, "LGUI"),
)


def _report_descriptor(path: str) -> bytes:
    try:
        fd = os.open(path, os.O_RDONLY)
    except OSError:
        return b""
    try:
        buf = bytearray(4)
        fcntl.ioctl(fd, HIDIOCGRDESCSIZE, buf)
        size = struct.unpack("<I", buf)[0]
        buf = bytearray(4 + 4096)
        struct.pack_into("<I", buf, 0, size)
        fcntl.ioctl(fd, HIDIOCGRDESC, buf)
        return bytes(buf[4:4 + size])
    except OSError:
        return b""
    finally:
        os.close(fd)


def _usb_ids(path: str) -> tuple[int, int] | None:
    uevent = Path(f"/sys/class/hidraw/{Path(path).name}/device/uevent")
    try:
        text = uevent.read_text()
    except OSError:
        return None
    for line in text.splitlines():
        if line.startswith("HID_ID="):
            parts = line.split("=", 1)[1].split(":")
            if len(parts) == 3:
                return int(parts[1], 16), int(parts[2], 16)
    return None


def find_raw_hid(vid: int, pid: int) -> str:
    for path in sorted(glob.glob("/dev/hidraw*")):
        if _usb_ids(path) != (vid, pid):
            continue
        if _report_descriptor(path).startswith(RAW_HID_USAGE):
            return path
    raise SystemExit(
        f"No VIA raw HID interface (usage page 0xFF60) for {vid:04X}:{pid:04X} found; "
        "connect the keyboard or pass a VIA backup JSON."
    )


class Via:
    def __init__(self, path: str):
        self.fd = os.open(path, os.O_RDWR)

    def transfer(self, payload: bytes) -> bytes:
        os.write(self.fd, payload.ljust(32, b"\0"))
        ready, _, _ = select.select([self.fd], [], [], 1.0)
        if not ready:
            raise SystemExit("No response from the VIA raw HID interface.")
        return os.read(self.fd, 64)

    def layer_count(self) -> int:
        return self.transfer(bytes([0x11]))[1]

    def reset(self) -> None:
        # id_dynamic_keymap_reset: dynamic_keymap_reset() copies keymaps[] from
        # flash into the emulated EEPROM, i.e. seeds the board from keymap.c.
        self.transfer(bytes([0x06]))

    def keycode(self, layer: int, row: int, col: int) -> int:
        response = self.transfer(bytes([0x04, layer, row, col]))
        return struct.unpack(">H", response[4:6])[0]


def load_keycodes(qmk_home: Path) -> dict[int, dict]:
    codes: dict[int, dict] = {}
    for path in sorted((qmk_home / "data/constants/keycodes").glob("keycodes_*.hjson")):
        try:
            data = json.loads(path.read_text())
        except (OSError, ValueError):
            continue  # HJSON-only files (comments) are not needed for the codes below
        for value, entry in data.get("keycodes", {}).items():
            try:
                number = int(value, 16)
            except ValueError:
                continue
            codes.setdefault(number, entry)
    if not codes:
        raise SystemExit(f"No keycode data under {qmk_home}/data/constants/keycodes")
    return codes


def _keycode_name(entry: dict) -> str:
    names = [entry["key"]] + entry.get("aliases", [])
    names = [n for n in names if n and set(n) != {"_"}]
    return min(names, key=len)


def render_keycode(value: int, codes: dict[int, dict]) -> str:
    if 0x0100 <= value <= 0x1FFF:
        mods, base = value >> 8, value & 0xFF
        entry = codes.get(base)
        if entry is None:
            return f"0x{value:04X}"
        name = _keycode_name(entry)
        right = bool(mods & 0x10)
        for bit, mod in MODS:
            if (bit & 0x10) == (0x10 if right else 0) and mods & (bit & 0x0F):
                name = f"{mod}({name})"
        return name
    if 0x5200 <= value <= 0x521F:
        return f"TO({value & 0x1F})"
    if 0x5220 <= value <= 0x523F:
        return f"MO({value & 0x1F})"
    if 0x5240 <= value <= 0x525F:
        return f"DF({value & 0x1F})"
    if 0x5260 <= value <= 0x527F:
        return f"TG({value & 0x1F})"
    if 0x5280 <= value <= 0x529F:
        return f"OSL({value & 0x1F})"
    if 0x52C0 <= value <= 0x52DF:
        return f"TT({value & 0x1F})"
    entry = codes.get(value)
    return _keycode_name(entry) if entry else f"0x{value:04X}"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-o", "--output", help="VIA backup JSON to write")
    parser.add_argument("--reset", action="store_true", help="reseed the board's dynamic keymap from the firmware (keymap.c) and exit, instead of dumping")
    parser.add_argument("--qmk-home", default=os.environ.get("QMK_HOME") or str(Path(__file__).resolve().parent / "qmk_firmware"))
    parser.add_argument("--rows", type=int, default=12)
    parser.add_argument("--cols", type=int, default=6)
    parser.add_argument("--name", default="do52pro")
    parser.add_argument("--vid", type=lambda v: int(v, 0), default=0x594B)
    parser.add_argument("--pid", type=lambda v: int(v, 0), default=0x0D52)
    parser.add_argument("--device", help="raw HID path (default: first matching usage page 0xFF60)")
    args = parser.parse_args()

    if not args.reset and not args.output:
        parser.error("-o/--output is required unless --reset is given")

    device = args.device or find_raw_hid(args.vid, args.pid)
    via = Via(device)

    if args.reset:
        via.reset()
        print(f"Reseeded the dynamic keymap from the firmware on {device}", file=sys.stderr)
        return 0

    codes = load_keycodes(Path(args.qmk_home))

    layers = []
    for layer in range(via.layer_count()):
        layer_keys = []
        for row in range(args.rows):
            for col in range(args.cols):
                layer_keys.append(render_keycode(via.keycode(layer, row, col), codes))
        layers.append(layer_keys)

    backup = {
        "name": args.name,
        "macros": [""] * 16,
        "layers": layers,
    }
    Path(args.output).write_text(json.dumps(backup, indent=2) + "\n")
    print(f"Dumped {len(layers)} layers from {device}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
