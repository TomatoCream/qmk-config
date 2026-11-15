#include QMK_KEYBOARD_H

// Keymap adapté depuis ../zmk-config/config/do52pro.keymap
// LAYOUT_5x6 order: left rows 0-4 (30), right rows 6-9 (24),
// right thumb row 10 (6), left DPAD (4), right DPAD (4) = 68 keys.
//
// Notes on the ZMK -> QMK translation:
//  - ZMK LBRC/RBRC = braces {}  -> QMK KC_LCBR/KC_RCBR
//  - ZMK LBKT/RBKT = brackets [] -> QMK KC_LBRC/KC_RBRC
//  - ZMK LPAR/RPAR = ()          -> QMK KC_LPRN/KC_RPRN
//  - ZMK &mo 1 -> MO(1), &to 0 -> TO(0)
//  - ZMK &sys_reset -> QK_RBT, &bootloader -> QK_BOOT
//  - ZMK &mkp LCLK -> KC_BTN1 (transmitted by process_record_user below)
//  - ZMK BLE (&bt ...), &ext_power are Bluetooth/nRF only -> KC_NO on this
//    wired RP2040 build.
//  - The physical bottom row has 5 keys/side in ZMK, but this LAYOUT_5x6
//    only exposes 4 DPAD keys/side, so C_PP (play/pause) and K_MUTE have
//    no slot and are dropped.
//
// Layer 0 is Dvorak. Caps Lock is TG(3), which overlays layer 3 (the QWERTY
// letter positions) on top of it; press again to return to Dvorak.

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // Layer 0: dvorak -- the base layout. Caps Lock is TG(3): it swaps the
    // 30 letter keys to QWERTY (layer 3) and back.
    [0] = LAYOUT_5x6(
        // Left rows 0-4
        KC_ESC,  KC_1,    KC_2,    KC_3,   KC_4,    KC_5,
        KC_TAB,  KC_QUOT, KC_COMM, KC_DOT, KC_P,    KC_Y,
        TG(3),   KC_A,    KC_O,    KC_E,   KC_U,    KC_I,
        KC_LSFT, KC_SCLN, KC_Q,    KC_J,   KC_K,    KC_X,
        MO(1),   KC_LGUI, KC_NO,   KC_NO,  KC_SPC,  KC_LGUI,
        // Right rows 6-10
        KC_6,    KC_7,    KC_8,    KC_9,   KC_0,    KC_BSPC,
        KC_F,    KC_G,    KC_C,    KC_R,   KC_L,    KC_BSLS,
        KC_D,    KC_H,    KC_T,    KC_N,   KC_S,    KC_ENT,
        KC_B,    KC_M,    KC_W,    KC_V,   KC_Z,    KC_RSFT,
        KC_BTN1, KC_BTN2, KC_NO,   KC_NO,  KC_LGUI, MO(1),
        // Left DPAD
        KC_MPRV, KC_VOLU,  KC_VOLD, KC_MNXT,
        // Right DPAD
        KC_LEFT, KC_RIGHT, KC_DOWN, KC_UP),
        // Left DPAD: LEFT DOWN UP RIGHT
        // Right DPAD: LEFT RIGHT DOWN UP

    // Layer 1: upper_layer (symbols)
    [1] = LAYOUT_5x6(
        // Left rows 0-4
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_TAB,  KC_NO,   KC_NO,   KC_LCBR, KC_RCBR, KC_GRV,
        KC_LSFT, KC_NO,   KC_NO,   KC_LPRN, KC_RPRN, KC_NO,
        KC_LSFT, KC_NO,   KC_NO,   KC_LPRN, KC_RPRN, KC_NO,
        KC_LCTL, KC_LGUI, KC_NO,   KC_NO,   KC_SPC,  KC_NO,
        // Right rows 6-10
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_LBRC, KC_RBRC, KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_LCBR, KC_RCBR, KC_NO,   KC_NO,   QK_RBT,
        KC_NO,   KC_LCBR, KC_RCBR, KC_NO,   KC_NO,   QK_BOOT,
        KC_TRNS, KC_SPC,  KC_NO,   KC_NO,   KC_LALT, KC_RGUI,
        // Left DPAD
        KC_NO, KC_BRID, KC_BRIU, KC_NO,
        // Right DPAD
        KC_NO, KC_BRID, KC_BRIU, KC_NO),

    // Layer 2: func_layer (F-keys, resets; BLE keys are no-ops here)
    [2] = LAYOUT_5x6(
        // Left rows 0-4
        KC_ESC, KC_F1, KC_F2,   KC_F3,   KC_F4,   KC_F5,
        KC_NO,  TO(0), KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,  KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,  KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,  KC_NO, KC_NO,   KC_NO,   KC_LALT, KC_RGUI,
        // Right rows 6-10
        KC_F6,  KC_F7, KC_F8,   KC_F9,   KC_F10,  KC_DEL,
        KC_NO,  KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,  KC_NO, KC_NO,   KC_NO,   KC_NO,   QK_RBT,
        KC_NO,  KC_NO, KC_LCTL, KC_LGUI, KC_NO,   QK_BOOT,
        KC_NO,  KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,
        // Left DPAD
        KC_NO, KC_NO, KC_NO, KC_NO,
        // Right DPAD
        KC_NO, KC_NO, KC_NO, KC_NO),

    // Layer 3: qwerty letter overlay, toggled by TG(3) (Caps Lock on layer 0).
    // Only the 30 letter keys are overridden; everything else is KC_TRNS and
    // falls through to layer 0, so numbers, modifiers, thumbs, DPAD and mouse
    // buttons stay defined in one place. Not a base layer: it is the 4th VIA
    // layer, so VIA edits it directly.
    [3] = LAYOUT_5x6(
        // Left rows 0-4
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,
        KC_TRNS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,
        KC_TRNS, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        // Right rows 6-10
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_TRNS,
        KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_TRNS,
        KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        // Left DPAD
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        // Right DPAD
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS)};

// PS2_MOUSE_ENABLE makes quantum's register_mouse() treat mouse button
// keycodes as pointing-device buttons and skip mousekey_send(); the legacy
// ps2_mouse driver only reports the TrackPoint's own buttons, so KC_BTN1..8
// from the keymap never reach the host. Send them ourselves.
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_BTN1 ... KC_BTN8:
            if (record->event.pressed) {
                mousekey_on((uint8_t)keycode);
            } else {
                mousekey_off((uint8_t)keycode);
            }
            mousekey_send();
            return false;
        default:
            return true;
    }
}
