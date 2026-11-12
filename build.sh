#!/usr/bin/env bash
# Build the DO52 Pro (RP2040) QMK firmware.
#
#   ./build.sh [keymap]                    # keymap defaults to "default"
#   QMK_HOME=/path/to/qmk_firmware ./build.sh
#
# Re-execs into the flake's dev shell when the ARM toolchain is not on PATH.
set -euo pipefail

here=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
keymap=${1:-default}
QMK_HOME=${QMK_HOME:-/home/df/archive/raptop/usb/df/projects/qmk_firmware}

if ! command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    exec nix develop "$here" --command "$here/build.sh" "$@"
fi

[ -f "$QMK_HOME/builddefs/common_features.mk" ] || {
    echo "QMK_HOME does not look like a qmk_firmware checkout: $QMK_HOME" >&2
    exit 1
}

# Install the keyboard definition into the QMK tree. Symlink, so edits here are
# picked up by later builds without re-copying.
ln -sfn "$here/keyboards/do52pro" "$QMK_HOME/keyboards/do52pro"

# Backport the either-pin-order PS/2 PIO driver. Idempotent: skip if the QMK
# checkout already has it (e.g. current master).
drv="$QMK_HOME/platforms/chibios/drivers/vendor/RP/RP2040/ps2_vendor.c"
if ! grep -q 'PS2_CLOCK_PINDIR_BIT' "$drv"; then
    echo "Applying PS/2 either-order patch..."
    patch -d "$QMK_HOME" -p1 < "$here/patches/ps2_vendor-either-order.patch"
fi

cd "$QMK_HOME"
qmk compile -kb do52pro -km "$keymap"

mkdir -p "$here/firmware-rp2040"
out="$here/firmware-rp2040/do52pro_trackpoint_${keymap}.uf2"
cp "$QMK_HOME/do52pro_${keymap}.uf2" "$out"
echo "Wrote $out"
