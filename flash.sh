#!/usr/bin/env bash
# Flash the built UF2 to whichever half is in BOOTSEL mode (RPI-RP2).
#
#   ./flash.sh [path/to/firmware.uf2]
#
# Both halves take the same firmware; repeat for the other half.
set -euo pipefail

here=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
uf2=${1:-}
if [ -z "$uf2" ]; then
    uf2=$(ls -t "$here"/firmware-rp2040/*.uf2 2>/dev/null | head -1 || true)
fi
if [ -z "$uf2" ] || [ ! -f "$uf2" ]; then
    echo "No UF2 found. Run ./build.sh first, or pass a path." >&2
    exit 1
fi

echo "Waiting for RPI-RP2 (double-tap RESET, or hold BOOT while plugging USB)..."
dev=""
for _ in $(seq 1 120); do
    dev=$(lsblk -rno NAME,LABEL | awk '$2=="RPI-RP2"{print $1; exit}')
    [ -n "$dev" ] && break
    sleep 1
done
if [ -z "$dev" ]; then
    echo "Timed out: no RPI-RP2 drive found." >&2
    exit 1
fi

mp=$(lsblk -rno MOUNTPOINT "/dev/$dev")
if [ -z "$mp" ]; then
    udisksctl mount -b "/dev/$dev" >/dev/null
    mp=$(lsblk -rno MOUNTPOINT "/dev/$dev")
fi
if [ -z "$mp" ]; then
    echo "Failed to mount /dev/$dev" >&2
    exit 1
fi

echo "Copying $(basename "$uf2") to $mp ..."
# The board reboots the instant the write finishes; cp/sync may report an error
# even on success.
cp "$uf2" "$mp"/ 2>/dev/null || true
sync 2>/dev/null || true
echo "Done. Unplug/replug USB to power-cycle the TrackPoint."
