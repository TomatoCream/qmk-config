# AGENTS.md

## Overview

QMK config for the DO52 Pro (RP2040 split, TrackPoint) plus a VIA draft
definition for the Keychron V10. The QMK checkout is a pinned submodule; only
the keyboard overlay, keymaps, VIA definitions, and tooling live here.

## Commands

- `./build.sh [keymap]` — symlink the overlay into `qmk_firmware`, apply the PS/2
  patch, compile, copy the UF2 to `firmware-rp2040/`. Keymap defaults to `default`.
- `./flash.sh [keymap|file.uf2]` — wait for the `RPI-RP2` BOOTSEL volume, then copy
  the newest UF2. BOOTSEL entry is physical.
- `./via-export.sh` — dump the running DO52 Pro's VIA EEPROM over raw HID and write
  formatted `./keymap.generated.c`.
- `./via-export.sh <via-backup.json> [out.c]` — convert a VIA *Save + Load* backup
  instead of the live EEPROM.
- `./via-pull.sh [do52|do52pro] [--check]` — dump the EEPROM and patch only changed
  positions in `keyboards/do52pro/keymaps/default/keymap.c`; `--check` reports and
  exits 1 when there are changes.
- `./via-dump.py -o out.json` — raw EEPROM to VIA backup JSON. Options: `--rows`,
  `--cols`, `--vid`, `--pid`, `--device`, `--qmk-home`; `--reset` reseeds the
  board's dynamic keymap from the firmware (VIA cmd `0x06`) instead of dumping.
- `./via-apply.py <backup.json> [--layer N] [--check]` — write a VIA backup JSON
  to the running board's EEPROM over raw HID, position by position; only values
  that differ are written and read back. `--check` reports the differences and
  exits 1. Same `--rows/--cols/--vid/--pid/--device/--qmk-home` options as
  `via-dump.py`. V10: `--vid 0x3434 --pid 0x03A1 --rows 6 --cols 16`.
- `./format-keymap.py <file>` / `--check` — reflow `LAYOUT_5x6(...)` calls into the
  table grid; `--check` exits non-zero if a file would change.
- Verification: `./build.sh` and `./format-keymap.py --check
  keyboards/do52pro/keymaps/default/keymap.c`.

## Layout

- `keyboards/do52pro/` — QMK keyboard definition, `keymaps/default/keymap.c`,
  `via/do52pro.json` (VIA Design-tab definition), `via/do52pro-live.layout.json`
  (committed snapshot for offline converter tests).
- `keyboards/keychron-v10/via/` — VIA Design-tab definition plus
  `keychron-v10-live.keymap.json`, a snapshot of the board's dynamic keymap
  (4 layers, 6×16). QMK hardware is upstream `keychron/v10/ansi_encoder`
  (`LAYOUT_ansi_89`, USB `3434:03A1`); VIA already knows the stock board. The
  board's keymap lives in its EEPROM — this repo holds no V10 `keymap.c`.
  Layer 0 (Mac base) and layer 2 (Windows base, selected by the rear Mac/Win
  slider) both carry Dvorak letters; layers 1 and 3 are the Fn layers.
  Re-apply the snapshot over raw HID with
  `./via-apply.py keyboards/keychron-v10/via/keychron-v10-live.keymap.json
  --vid 0x3434 --pid 0x03A1 --rows 6 --cols 16`; re-dump it after remapping in
  VIA.
- `qmk_firmware/` — pinned QMK submodule; `build.sh` removes its symlink and reverts
  the PS/2 patch after every build.
- `via-export.sh`, `via-dump.py`, `via-merge.py`, `via-pull.sh`,
  `format-keymap.py` — keymap tooling. `flash.sh`/`build.sh` do not use the VIA
  definitions.

## Conventions

- `keymap.c` is source; VIA writes only the EEPROM. After remapping in VIA, run
  `./via-pull.sh`; do not hand-copy the generated table.
- `via-pull` maps positions by `LAYOUT_5x6` argument index: argument N is the same
  physical key in every layer (`keyboard.json` layout order). It compares resolved
  keycode values, so `KC_LCBR` == `LSFT(KC_LBRC)` and `KC_RIGHT` == `KC_RGHT`; only
  differing arguments are replaced, preserving comments, formatting, and
  `process_record_user()`.
- The dynamic keymap has 4 VIA layers and the C keymap fills all four: 0 =
  Dvorak base, 1 = symbols, 2 = F-keys/reset (unreachable from 0/1), 3 =
  QWERTY letter overlay. All four are pulled by `via-pull.sh`.
- Layer 0 is Dvorak (`' , . p y / a o e u i / ; q j k x / f g c r l / d h t n s
  / b m w v z` on the 30 letter positions). Caps Lock is `TG(3)`, toggling
  layer 3, which overrides only those 30 letter positions with QWERTY and is
  `KC_TRNS` elsewhere — never duplicate non-letter keys into layer 3. Raw
  keycode for VIA's *Any* field: `TG(3)` = `0x5263`.
- Raw HID discovery filters USB ID `0x594B:0x0D52`; other VIA keyboards (the
  Keychron V10 at `0x3434:0x03A1`) are ignored.
- `format-keymap.py` replaces comments inside `LAYOUT_5x6(...)` with group
  headings; keep notes outside the calls.
- `KC_BTN1..8` are transmitted by `process_record_user()` in `keymap.c` because
  `PS2_MOUSE_ENABLE` makes QMK skip `mousekey_send()`. `ps2_mouse_moved_user()` in
  `do52pro.c` merges held buttons into TrackPoint movement reports so drag works.
- VIA definition `vendorId`/`productId` must match `config.h` (`0x594B:0x0D52`).

## Gotchas

- The EEPROM dynamic keymap is seeded from `keymap.c` only on first boot. Flashing
  does not clear it; after flashing a `keymap.c` change run
  `./via-dump.py --reset` (VIA cmd `0x06` over raw HID, no BOOTSEL). A layer only
  present in the C keymap (e.g. the QWERTY overlay) stays all-`KC_TRNS` in the
  EEPROM until then, so the new `TG(3)` would appear to do nothing — and
  `via-pull.sh` would write those `KC_TRNS` back into `keymap.c`. Both reseed
  paths discard VIA-only remaps, so run `./via-pull.sh` first to keep them.
  Bootmagic fallback: hold the right half's `6` (`[6,0]`) while plugging in.
- USB must be on the right half (`MASTER_RIGHT`); connect TRRS before USB.
- `flash.sh` needs BOOTSEL (double-tap RESET or hold BOOT at plug-in). VIA's
  `id_bootloader_jump` is not implemented in this firmware, so there is no software
  reset.
- After flashing, unplug/replug to power-cycle the TrackPoint.
- `build.sh` re-execs into `nix develop` when `arm-none-eabi-gcc` is missing;
  `QMK_HOME` overrides the QMK checkout.
- `via-dump.py` needs `$QMK_HOME/data/constants/keycodes`; unknown keycodes fall
  back to raw `0xNNNN` C literals.
- `via/do52pro-live.layout.json` is a snapshot, not the live state; use
  `./via-export.sh` (no argument) for the board's current keymap.
