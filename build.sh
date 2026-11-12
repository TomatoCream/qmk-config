#!/usr/bin/env bash
# Build the DO52 Pro (RP2040) QMK firmware.
#
#   ./build.sh [keymap]                    # keymap defaults to "default"
#   QMK_HOME=/path/to/qmk_firmware ./build.sh
#
# QMK lives in the git submodule ./qmk_firmware (pinned commit). Re-execs into
# the flake's dev shell when the ARM toolchain is not on PATH.
set -euo pipefail

here=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
keymap=${1:-default}
default_qmk="$here/qmk_firmware"
if [ ! -d "$default_qmk" ]; then
    default_qmk=/home/df/archive/raptop/usb/df/projects/qmk_firmware
fi
QMK_HOME=${QMK_HOME:-$default_qmk}

if ! command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    exec nix develop "$here" --command "$here/build.sh" "$@"
fi

[ -f "$QMK_HOME/builddefs/common_features.mk" ] || {
    echo "QMK_HOME does not look like a qmk_firmware checkout: $QMK_HOME" >&2
    echo "Initialize the submodule: git submodule update --init --recursive" >&2
    exit 1
}

drv_rel=platforms/chibios/drivers/vendor/RP/RP2040/ps2_vendor.c
kb_link="$QMK_HOME/keyboards/do52pro"
patched=0

# Keep the QMK submodule clean: revert the local driver patch, drop the
# keyboard symlink and the in-tree UF2 after the build (success or failure).
cleanup() {
    if [ "$patched" = 1 ] && [ -e "$QMK_HOME/.git" ]; then
        git -C "$QMK_HOME" checkout -- "$drv_rel" 2>/dev/null || true
    fi
    rm -f "$kb_link" "$QMK_HOME/do52pro_${keymap}.uf2"
}
trap cleanup EXIT

ln -sfn "$here/keyboards/do52pro" "$kb_link"

# Backport the either-pin-order PS/2 PIO driver. Skip when the checkout already
# has it (current QMK master).
if ! grep -q 'PS2_CLOCK_PINDIR_BIT' "$QMK_HOME/$drv_rel"; then
    echo "Applying PS/2 either-order patch..."
    patch -d "$QMK_HOME" -p1 < "$here/patches/ps2_vendor-either-order.patch"
    patched=1
fi

cd "$QMK_HOME"
qmk compile -kb do52pro -km "$keymap"

mkdir -p "$here/firmware-rp2040"
out="$here/firmware-rp2040/do52pro_trackpoint_${keymap}.uf2"
cp "$QMK_HOME/do52pro_${keymap}.uf2" "$out"
echo "Wrote $out"
