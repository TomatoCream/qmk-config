#!/usr/bin/env bash
# Turn a VIA "Save + Load" backup into a keymap.c.
#
#   ./via-export.sh ~/Downloads/do52pro.layout.json            # -> ./keymap.generated.c
#   ./via-export.sh backup.json keyboards/do52pro/keymaps/default/keymap.c
#   QMK_HOME=/path/to/qmk_firmware ./via-export.sh backup.json
#
# VIA stores layers in matrix order (12x6). `qmk via2json` maps them back to the
# LAYOUT_5x6 argument order, `qmk json2c` writes the C table. Review the result
# (it drops the comments in keymap.c) before replacing the default keymap.
set -euo pipefail

here=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
keyboard=do52pro
layout=LAYOUT_5x6

[ $# -ge 1 ] || { echo "usage: $0 <via-backup.json> [output.c]" >&2; exit 2; }
backup=$(realpath -- "$1")
out=$(realpath -m -- "${2:-$here/keymap.generated.c}")
QMK_HOME=${QMK_HOME:-$here/qmk_firmware}

[ -f "$backup" ] || { echo "No such file: $backup" >&2; exit 1; }
[ -f "$QMK_HOME/builddefs/common_features.mk" ] || {
    echo "QMK_HOME does not look like a qmk_firmware checkout: $QMK_HOME" >&2
    echo "Initialize the submodule: git submodule update --init --recursive" >&2
    exit 1
}

if ! command -v qmk >/dev/null 2>&1; then
    exec nix develop "$here" --command "$here/via-export.sh" "$@"
fi

kb_link="$QMK_HOME/keyboards/$keyboard"
keymap_json=$(mktemp --suffix=.json)
trap 'rm -f "$keymap_json" "$kb_link"' EXIT

# qmk resolves the keyboard (keyboard.json, matrix size) from inside the tree
# and only exposes via2json/json2c when run from the QMK root.
ln -sfn "$here/keyboards/$keyboard" "$kb_link"
cd "$QMK_HOME"
qmk via2json -q -kb "$keyboard" -km via -l "$layout" -o "$keymap_json" "$backup"
qmk json2c -q -o "$out" "$keymap_json"

echo "Wrote $out"
