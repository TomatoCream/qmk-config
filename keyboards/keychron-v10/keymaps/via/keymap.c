/* Copyright 2023 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H
#include "keychron_common.h"

// Layer 0/2 are the Dvorak Mac/Windows bases (rear dip switch picks one);
// layer 4 is a QWERTY overlay toggled by TG(4) on the M4 macro key
// [4,0]. Everything else on layer 4 is transparent, so the overlay follows
// whichever base layer is active. Layers 1/3 are the Fn layers.
enum layers {
    MAC_BASE,
    MAC_FN,
    WIN_BASE,
    WIN_FN,
    QWERTY,
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [MAC_BASE] = LAYOUT_ansi_89(
        KC_MUTE , KC_ESC  , KC_BRID , KC_BRIU , 0x7F00  , 0x7F01  , RGB_VAD , RGB_VAI , KC_MPRV , KC_MPLY , KC_MNXT , KC_MUTE , KC_VOLD , KC_VOLU , KC_INS  , KC_DEL,
        MC_3    , KC_GRV  , KC_1    , KC_2    , KC_3    , KC_4    , KC_5    , KC_6    , KC_7    , KC_8    , KC_9    , KC_0    , KC_LBRC , KC_RBRC , KC_BSPC , KC_PGUP,
        MC_4    , KC_TAB  , KC_QUOT , KC_COMM , KC_DOT  , KC_P    , KC_Y    , KC_F    , KC_G    , KC_C    , KC_R    , KC_L    , KC_SLSH , KC_EQL  , KC_BSLS , KC_PGDN,
        MC_5    , KC_CAPS , KC_A    , KC_O    , KC_E    , KC_U    , KC_I    , KC_D    , KC_H    , KC_T    , KC_N    , KC_S    , KC_MINS , KC_ENT  , KC_HOME,
        TG(4)   , KC_LSFT , KC_SCLN , KC_Q    , KC_J    , KC_K    , KC_X    , KC_X    , KC_B    , KC_M    , KC_W    , KC_V    , KC_Z    , KC_RSFT , KC_UP,
        KC_PSCR , KC_LCTL , 0x7F02  , 0x7F04  , KC_SPC  , MO(1)   , KC_SPC  , 0x7F05  , KC_LEFT , KC_DOWN , KC_RGHT),

    [MAC_FN] = LAYOUT_ansi_89(
        RGB_TOG , KC_TRNS , KC_F1   , KC_F2   , KC_F3   , KC_F4   , KC_F5   , KC_F6   , KC_F7   , KC_F8   , KC_F9   , KC_F10  , KC_F11  , KC_F12  , KC_TRNS , KC_TRNS,
        KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS,
        KC_TRNS , RGB_TOG , RGB_MOD , RGB_VAI , RGB_HUI , RGB_SAI , RGB_SPI , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS,
        KC_TRNS , KC_TRNS , RGB_RMOD, RGB_VAD , RGB_HUD , RGB_SAD , RGB_SPD , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS,
        KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , NK_TOGG , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS,
        KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS),

    [WIN_BASE] = LAYOUT_ansi_89(
        KC_MUTE , KC_ESC  , KC_F1   , KC_F2   , KC_F3   , KC_F4   , KC_F5   , KC_F6   , KC_F7   , KC_F8   , KC_F9   , KC_F10  , KC_F11  , KC_F12  , KC_INS  , KC_DEL,
        MC_3    , KC_GRV  , KC_1    , KC_2    , KC_3    , KC_4    , KC_5    , KC_6    , KC_7    , KC_8    , KC_9    , KC_0    , KC_LBRC , KC_RBRC , KC_BSPC , KC_PGUP,
        MC_4    , KC_TAB  , KC_QUOT , KC_COMM , KC_DOT  , KC_P    , KC_Y    , KC_F    , KC_G    , KC_C    , KC_R    , KC_L    , KC_SLSH , KC_EQL  , KC_BSLS , KC_PGDN,
        MC_5    , KC_CAPS , KC_A    , KC_O    , KC_E    , KC_U    , KC_I    , KC_D    , KC_H    , KC_T    , KC_N    , KC_S    , KC_MINS , KC_ENT  , KC_HOME,
        TG(4)   , KC_LSFT , KC_SCLN , KC_Q    , KC_J    , KC_K    , KC_X    , KC_X    , KC_B    , KC_M    , KC_W    , KC_V    , KC_Z    , KC_RSFT , KC_UP,
        KC_PSCR , KC_LCTL , KC_LGUI , KC_LALT , KC_SPC  , MO(3)   , KC_SPC  , KC_RALT , KC_LEFT , KC_DOWN , KC_RGHT),

    [WIN_FN] = LAYOUT_ansi_89(
        RGB_TOG , KC_TRNS , KC_BRID , KC_BRIU , 0x7F07  , 0x7F08  , RGB_VAD , RGB_VAI , KC_MPRV , KC_MPLY , KC_MNXT , KC_MUTE , KC_VOLD , KC_VOLU , KC_TRNS , KC_TRNS,
        KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS,
        KC_TRNS , RGB_TOG , RGB_MOD , RGB_VAI , RGB_HUI , RGB_SAI , RGB_SPI , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS,
        KC_TRNS , KC_TRNS , RGB_RMOD, RGB_VAD , RGB_HUD , RGB_SAD , RGB_SPD , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS,
        KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , NK_TOGG , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS,
        KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS),

    [QWERTY] = LAYOUT_ansi_89(
        KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS,
        KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_MINS , KC_EQL  , KC_TRNS , KC_TRNS,
        KC_TRNS , KC_TRNS , KC_Q    , KC_W    , KC_E    , KC_R    , KC_T    , KC_Y    , KC_U    , KC_I    , KC_O    , KC_P    , KC_LBRC , KC_RBRC , KC_TRNS , KC_TRNS,
        KC_TRNS , KC_TRNS , KC_A    , KC_S    , KC_D    , KC_F    , KC_G    , KC_H    , KC_J    , KC_K    , KC_L    , KC_SCLN , KC_QUOT , KC_TRNS , KC_TRNS,
        KC_TRNS , KC_TRNS , KC_Z    , KC_X    , KC_C    , KC_V    , KC_B    , KC_B    , KC_N    , KC_M    , KC_COMM , KC_DOT  , KC_SLSH , KC_TRNS , KC_TRNS,
        KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS , KC_TRNS),
};
// clang-format on

#if defined(ENCODER_ENABLE) && defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [MAC_BASE] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [MAC_FN]   = {ENCODER_CCW_CW(RGB_VAD, RGB_VAI)},
    [WIN_BASE] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [WIN_FN]   = {ENCODER_CCW_CW(RGB_VAD, RGB_VAI)},
    [QWERTY]   = {ENCODER_CCW_CW(KC_TRNS, KC_TRNS)},
};
#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // The stock firmware stores Keychron's custom keycodes in the old QK_USER
    // range (0x7F00); QK_KB_* has since moved to 0x7E00. Handle both so
    // values already in the EEPROM keep working after the flash.
    if (keycode >= 0x7F00 && keycode < 0x7F00 + 0x20) {
        process_record_keychron(QK_KB_0 + (keycode - 0x7F00), record);
        return false;
    }
    return process_record_keychron(keycode, record);
}

void housekeeping_task_user(void) {
    housekeeping_task_keychron();
}
