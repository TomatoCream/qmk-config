#include "do52pro.h"
#include "print.h"
#include "debug.h"

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
