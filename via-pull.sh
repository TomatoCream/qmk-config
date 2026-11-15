#!/usr/bin/env bash
# Pull the live VIA dynamic keymap into keymap.c, patching changed positions.
#
#   ./via-pull.sh            # do52pro: dump EEPROM, update changed keys
#   ./via-pull.sh do52       # same
#   ./via-pull.sh --check    # report changed positions, write nothing
set -euo pipefail

here=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
target="$here/keyboards/do52pro/keymaps/default/keymap.c"

keyboard=do52pro
check=0
for arg in "$@"; do
    case "$arg" in
        do52|do52pro) keyboard=$arg ;;
        --check) check=1 ;;
        *) echo "usage: $0 [do52|do52pro] [--check]" >&2; exit 2 ;;
    esac
done

"$here/via-export.sh" >/dev/null

if [ "$check" = 1 ]; then
    exec python3 "$here/via-merge.py" --check
fi

python3 "$here/via-merge.py"
"$here/format-keymap.py" "$target"
