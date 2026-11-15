"""Patch a keymap.c from a generated (live VIA) keymap, position by position.

Both files use LAYOUT_5x6; argument N is the same physical key in every layer
(keyboard.json's layout order). Keycodes are compared by resolved value, so
textual aliases (KC_LCBR vs LSFT(KC_LBRC), KC_RGHT vs KC_RIGHT) are equal and
do not get rewritten. Only calls with a real value change are replaced; the
header, comments between layers, and custom functions stay untouched.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

from . import QMK_HOME, ROOT, format_keymap as fk

MODBITS = {
    "LCTL": 0x01,
    "LSFT": 0x02,
    "LALT": 0x04,
    "LGUI": 0x08,
    "RCTL": 0x11,
    "RSFT": 0x12,
    "RALT": 0x14,
    "RGUI": 0x18,
}


def load_names(qmk_home: Path) -> dict[str, int]:
    names: dict[str, int] = {}
    for path in sorted((qmk_home / "data/constants/keycodes").glob("keycodes_*.hjson")):
        try:
            data = json.loads(path.read_text())
        except (OSError, ValueError):
            continue
        for value, entry in data.get("keycodes", {}).items():
            try:
                number = int(value, 16)
            except ValueError:
                continue
            for name in [entry["key"]] + entry.get("aliases", []):
                names.setdefault(name, number)

    # keymap_extras aliases such as KC_LCBR = S(KC_LEFT_BRACKET)
    extras = qmk_home / "quantum/keymap_extras/keymap_us.h"
    if extras.exists():
        for line in extras.read_text().splitlines():
            match = re.match(r"#define\s+(KC_\w+)\s+(.+?)\s*(?://.*)?$", line.strip())
            if match:
                names.setdefault(match.group(1), resolve(match.group(2), names))
    return names


def resolve(expr: str, names: dict[str, int], depth: int = 0) -> int | None:
    expr = expr.strip()
    if depth > 10:
        return None
    if re.fullmatch(r"0x[0-9a-fA-F]+", expr):
        return int(expr, 16)
    if re.fullmatch(r"\d+", expr):
        return int(expr)
    match = re.fullmatch(r"(\w+)\s*\((.*)\)", expr)
    if match:
        func, arg = match.group(1), match.group(2)
        if func in ("S", "LSFT"):
            inner = resolve(arg, names, depth + 1)
            return None if inner is None else 0x0200 | inner
        if func in MODBITS:
            inner = resolve(arg, names, depth + 1)
            return None if inner is None else (MODBITS[func] << 8) | inner
        if func in ("MO", "TO", "DF", "TG", "OSL", "TT"):
            base = {"MO": 0x5220, "TO": 0x5200, "DF": 0x5240, "TG": 0x5260, "OSL": 0x5280, "TT": 0x52C0}[func]
            inner = resolve(arg, names, depth + 1)
            return None if inner is None else base | inner
        if func == "LT":
            layer, _, key = arg.partition(",")
            layer_value = resolve(layer, names, depth + 1)
            inner = resolve(key, names, depth + 1)
            return None if layer_value is None or inner is None else 0x4000 | (layer_value << 8) | inner
        return None
    return names.get(expr)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="kb merge", description=__doc__)
    parser.add_argument("generated", help="live/generated keymap.c")
    parser.add_argument("target", nargs="?", default=str(ROOT / "keyboards/do52pro/keymaps/default/keymap.c"), help="keymap.c to patch")
    parser.add_argument("--qmk-home", default=str(QMK_HOME))
    parser.add_argument("--keyboard-json", default=str(ROOT / "keyboards/do52pro/keyboard.json"))
    parser.add_argument("--check", action="store_true", help="report changes without writing")
    args = parser.parse_args(argv)

    names = load_names(Path(args.qmk_home))

    generated_text = Path(args.generated).read_text()
    target_text = Path(args.target).read_text()
    generated = [args for _, _, args, _ in fk._find_calls(generated_text)]
    target_calls = fk._find_calls(target_text)

    layout = json.loads(Path(args.keyboard_json).read_text())["layouts"]["LAYOUT_5x6"]["layout"]

    print(f"generated: {len(generated)} layer(s); target: {len(target_calls)} layer(s)")
    replacements = []
    changes = 0

    for layer, (_, open_paren, target_args, close_paren) in enumerate(target_calls):
        if layer >= len(generated):
            break
        dynamic_args = generated[layer]
        changed_positions = set()
        for pos, (current, wanted) in enumerate(zip(target_args, dynamic_args)):
            if pos >= len(layout):
                break
            current_value = resolve(current, names)
            wanted_value = resolve(wanted, names)
            if current_value is None or wanted_value is None:
                same = current == wanted
            else:
                same = current_value == wanted_value
            if not same:
                matrix = layout[pos]["matrix"]
                print(f"  layer {layer} #{pos} [{matrix[0]},{matrix[1]}]: {current} -> {wanted}")
                changed_positions.add(pos)
                changes += 1
        if changed_positions:
            merged_args = [dynamic_args[pos] if pos in changed_positions else current for pos, current in enumerate(target_args)]
            replacements.append((open_paren + 1, close_paren, ", ".join(merged_args)))

    if len(generated) > len(target_calls):
        extra = generated[len(target_calls):]
        if any(key != "KC_TRNS" for layer in extra for key in layer):
            print(f"warning: generated has {len(extra)} extra non-transparent layer(s); not added", file=sys.stderr)

    if not changes:
        print("no key changes")
        return 0

    if args.check:
        print(f"{changes} change(s); --check, not writing")
        return 1

    for start, end, new_args in sorted(replacements, reverse=True):
        target_text = target_text[:start] + new_args + target_text[end:]
    Path(args.target).write_text(target_text)
    print(f"updated {args.target}")
    return 0
