;;; Directory Local Variables            -*- no-byte-compile: t; -*-
;; QMK keymaps are hand-aligned; formatters reflow them.
((nil . ((apheleia-inhibit . t)
         ;; clangd's on-type formatting triggers on newline and rewrites
         ;; LAYOUT_* macro calls while typing.
         (lsp-enable-on-type-formatting . nil))))
