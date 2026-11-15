"""Keymap and firmware tooling behind ./kb."""

from __future__ import annotations

import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
QMK_HOME = Path(os.environ.get("QMK_HOME") or ROOT / "qmk_firmware")
