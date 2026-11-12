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
//  - ZMK &mkp LCLK -> KC_BTN1 (needs MOUSEKEY_ENABLE to actually click)
//  - ZMK BLE (&bt ...), &ext_power are Bluetooth/nRF only -> KC_NO on this
//    wired RP2040 build.
//  - The physical bottom row has 5 keys/side in ZMK, but this LAYOUT_5x6
//    only exposes 4 DPAD keys/side, so C_PP (play/pause) and K_MUTE have
//    no slot and are dropped.

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // Layer 0: qwerty
    [0] = LAYOUT_5x6(
        // Left rows 0-4
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,
        MO(1),   KC_LGUI, KC_NO,   KC_NO,   KC_SPC,  KC_SPC,
        // Right rows 6-9
        KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,
        KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSLS,
        KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_ENT,
        KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
        // Right thumb row 10
        KC_BTN1, KC_ENT,  KC_NO,   KC_NO,   KC_LGUI, MO(1),
        // Left DPAD
        KC_MPRV, KC_VOLD, KC_VOLU, KC_MNXT,
        // Right DPAD
        KC_LEFT, KC_DOWN, KC_UP,  KC_RIGHT
    ),

    // Layer 1: upper_layer (symbols)
    [1] = LAYOUT_5x6(
        // Left rows 0-4
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_TAB,  KC_NO,   KC_NO,   KC_LCBR, KC_RCBR, KC_GRV,
        KC_LSFT, KC_NO,   KC_NO,   KC_LPRN, KC_RPRN, KC_NO,
        KC_LSFT, KC_NO,   KC_NO,   KC_LPRN, KC_RPRN, KC_NO,
        KC_LCTL, KC_LGUI, KC_NO,   KC_NO,   KC_SPC,  KC_NO,
        // Right rows 6-9
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_LBRC, KC_RBRC, KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_LCBR, KC_RCBR, KC_NO,   KC_NO,   QK_RBT,
        KC_NO,   KC_LCBR, KC_RCBR, KC_NO,   KC_NO,   QK_BOOT,
        // Right thumb row 10
        KC_TRNS, KC_SPC,  KC_NO,   KC_NO,   KC_LALT, KC_RGUI,
        // Left DPAD
        KC_NO,   KC_BRID, KC_BRIU, KC_NO,
        // Right DPAD
        KC_NO,   KC_BRID, KC_BRIU, KC_NO
    ),

    // Layer 2: func_layer (F-keys, resets; BLE keys are no-ops here)
    [2] = LAYOUT_5x6(
        // Left rows 0-4
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,
        KC_NO,   TO(0),   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_LALT, KC_RGUI,
        // Right rows 6-9
        KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_DEL,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   QK_RBT,
        KC_NO,   KC_NO,   KC_LCTL, KC_LGUI, KC_NO,   QK_BOOT,
        // Right thumb row 10
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        // Left DPAD
        KC_NO,   KC_NO,   KC_NO,   KC_NO,
        // Right DPAD
        KC_NO,   KC_NO,   KC_NO,   KC_NO
    )
};
