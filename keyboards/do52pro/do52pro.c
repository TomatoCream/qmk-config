#include "do52pro.h"
#include "print.h"
#include "debug.h"
#ifdef PS2_MOUSE_ENABLE
#    include "ps2_mouse.h"
#endif
#ifdef MOUSEKEY_ENABLE
#    include "mousekey.h"
#endif

// The legacy ps2_mouse driver builds every movement report from the
// TrackPoint's own button byte, which clears a keyboard-bound KC_BTN1..8 held
// on the thumb row. Merge it back so press-and-hold drag keeps working while
// the pointer moves.
#ifdef PS2_MOUSE_ENABLE
void ps2_mouse_moved_user(report_mouse_t *mouse_report) {
#    ifdef MOUSEKEY_ENABLE
    mouse_report->buttons |= mousekey_get_report().buttons;
#    endif
}
#endif

void keyboard_post_init_kb(void) {
#ifdef CONSOLE_ENABLE
    debug_enable = true;
    debug_mouse  = true;
    uprintf("do52pro KB init - Master: %s, Left: %s\n",
            is_keyboard_master() ? "YES" : "NO",
            is_keyboard_left() ? "YES" : "NO");
    // Do NOT reset the TrackPoint here. ps2_mouse_init() already did
    // 0xFF -> AA/00 -> 0xF4 (enable) -> 0xF0 (stream). A second reset
    // leaves the TrackPoint with data reporting disabled, so it never
    // sends another packet.
#endif
    keyboard_post_init_user();
}
