SERIAL_DRIVER = vendor

# VIA protocol + dynamic keymap. There is a single keymap (default), so VIA is
# enabled for the keyboard instead of through a separate keymaps/via wrapper.
VIA_ENABLE = yes

EXTRAKEY_ENABLE = yes

# KC_BTN1..8 need MOUSEKEY_ENABLE. PS2_MOUSE_ENABLE suppresses quantum's own
# button send, so keymap.c::process_record_user() transmits them.
MOUSEKEY_ENABLE = yes

# Debug console (`qmk console`). Enable while debugging PS/2 or boot.
# CONSOLE_ENABLE = yes
