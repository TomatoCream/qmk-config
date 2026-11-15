#!/usr/bin/env bash
# Turn the live VIA dynamic keymap (or a VIA backup) into a keymap.c.
#
#   ./via-export.sh                                   # dump the keyboard's EEPROM -> ./keymap.generated.c
#   ./via-export.sh ~/Downloads/do52pro.layout.json   # backup -> ./keymap.generated.c
#   ./via-export.sh backup.json keyboards/do52pro/keymaps/default/keymap.c
#   QMK_HOME=/path/to/qmk_firmware ./via-export.sh
#
# With no file argument, via-dump.py reads the connected keyboard's dynamic
# keymap over VIA raw HID (usage page 0xFF60) and writes a temporary VIA
# backup. VIA stores layers in matrix order (12x6); `qmk via2json` maps them
# back to the LAYOUT_5x6 argument order, `qmk json2c` writes the C table, and
# format-keymap.py reflows it into the same grid as keymap.c. Review the
# result before replacing the default keymap.
set -euo pipefail

here=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
keyboard=do52pro
layout=LAYOUT_5x6
QMK_HOME=${QMK_HOME:-$here/qmk_firmware}

[ -f "$QMK_HOME/builddefs/common_features.mk" ] || {
    echo "QMK_HOME does not look like a qmk_firmware checkout: $QMK_HOME" >&2
    echo "Initialize the submodule: git submodule update --init --recursive" >&2
    exit 1
}

if ! command -v qmk >/dev/null 2>&1; then
    exec nix develop "$here" --command "$here/via-export.sh" "$@"
fi

kb_link="$QMK_HOME/keyboards/$keyboard"
keymap_json=""
tmp_backup=""
cleanup() {
    rm -f "$kb_link"
    [ -n "$keymap_json" ] && rm -f "$keymap_json"
    [ -n "$tmp_backup" ] && rm -f "$tmp_backup"
    return 0
}
trap cleanup EXIT

if [ $# -ge 1 ]; then
    backup=$(realpath -- "$1")
else
    tmp_backup=$(mktemp --suffix=.layout.json)
    backup=$tmp_backup
    "$here/via-dump.py" --qmk-home "$QMK_HOME" -o "$backup"
fi
out=$(realpath -m -- "${2:-$here/keymap.generated.c}")

[ -f "$backup" ] || { echo "No such file: $backup" >&2; exit 1; }

keymap_json=$(mktemp --suffix=.json)

# qmk resolves the keyboard (keyboard.json, matrix size) from inside the tree
# and only exposes via2json/json2c when run from the QMK root.
ln -sfn "$here/keyboards/$keyboard" "$kb_link"
cd "$QMK_HOME"
qmk via2json -q -kb "$keyboard" -km default -l "$layout" -o "$keymap_json" "$backup"
qmk json2c -q -o "$out" "$keymap_json"
"$here/format-keymap.py" "$out"

echo "Wrote $out"
