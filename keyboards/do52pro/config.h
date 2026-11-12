#pragma once

// USB Device descriptor parameters
#define VENDOR_ID    0xFEED  // Valeur par défaut QMK, change si nécessaire
#define PRODUCT_ID  0x0000  // Valeur par défaut QMK, change si nécessaire
#define DEVICE_VER   0x0001  // Version du firmware
#define MANUFACTURER "YK"
#define PRODUCT      "do52pro"

// SERIAL_PIN est défini dans keyboard.json, pas besoin de le mettre ici

// NOTE: GP28 is used as matrix rows[5] in keyboard.json. Do not reuse it as
// DPAD_PIN — the double-assignment made the RP2040 read a phantom stuck key.

// Matrix rows 0 and 3 are GP27/GP29 (RP2040 ADC pins GP26-29). QMK's default
// unselect parks a row with a weak internal pull-up, which those pins do not
// hold high — so a pressed key kept its whole column driven low, ghosting the
// entire column across all rows. Actively drive unselected rows HIGH instead.
#define MATRIX_UNSELECT_DRIVE_HIGH

// The PS/2 TrackPoint (GP22/GP23) is wired to the right half. QMK's legacy
// ps2_mouse driver has no split-transport support — it calls host_mouse_send()
// straight from whichever half runs the task, and only the master half's
// reports reach the host. So the right half must be master (USB-connected).
#define MASTER_RIGHT
// #define MASTER_LEFT

// SERIAL_DRIVER = vendor (rules.mk) uses RP2040 PIO0 for split comms. The PS/2
// "vendor" driver (also RP2040-PIO-only — it's the only PS2 driver that
// compiles on RP2040, since ps2_io.c's open-drain mode isn't implemented in
// ChibiOS's RP2040 PAL) defaults to PIO0 too, and both register the same IRQ
// vector — linker fails with "multiple definition of Vector5C". Force PS/2
// onto PIO1 to free up PIO0 for the split transport.
#define PS2_PIO_USE_PIO1

// Stream mode: TrackPoint pushes data asynchronously, PIO captures it into a
// buffer. ps2_mouse_task() reads from the buffer without blocking the matrix
// scan — unlike remote mode which sends READ_DATA and blocks waiting for a
// response each scan cycle, causing severe key input lag.
// #define PS2_MOUSE_USE_REMOTE_MODE

// Orient the TrackPoint output to the physical stick mounting. This define is
// driver-independent (handled in QMK's ps2_mouse.c), so it applies to the RP2040
// vendor driver too. Valid values: 0, 90, 180, 270. This board needs 90 for the
// mount; if both axes feel reversed (up=down, left=right), add 180 -> 270.
#define PS2_MOUSE_ROTATE 270

// Désactiver les messages de débogage du transport série
// #define SERIAL_DEBUG
