#!/usr/bin/env python3
"""Reflow LAYOUT_5x6(...) calls into a fixed visual grid.

The do52pro LAYOUT_5x6 argument order (keyboard.json) is:

    0-29   left half, 5 rows of 6
    30-59  right half, 5 rows of 6 (last row is the thumb row)
    60-63  left DPAD
    64-67  right DPAD

Every call is rewritten in place; comments inside the argument list are
replaced by the group headings below. Use --check to test without writing.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

MACRO = "LAYOUT_5x6"
GROUPS = (
    ("Left rows 0-4", (6, 6, 6, 6, 6)),
    ("Right rows 6-10", (6, 6, 6, 6, 6)),
    ("Left DPAD", (4,)),
    ("Right DPAD", (4,)),
)
ARG_COUNT = sum(sum(rows) for _, rows in GROUPS)


def _skip_quoted(text: str, i: int) -> int:
    """Return the index just past the string/char literal starting at I."""
    quote = text[i]
    i += 1
    while i < len(text):
        if text[i] == "\\":
            i += 2
        elif text[i] == quote:
            return i + 1
        else:
            i += 1
    raise ValueError("unterminated string or character literal")


def _parse_args(text: str, open_paren: int) -> tuple[list[str], int]:
    """Split the call arguments and return (args, index of the closing paren)."""
    i = open_paren + 1
    depth = 1
    args: list[str] = []
    current: list[str] = []
    seen = False

    def finish() -> None:
        arg = "".join(current).strip()
        if not arg:
            raise ValueError("empty macro argument")
        args.append(arg)

    while i < len(text):
        c = text[i]
        if c == "/" and text.startswith("//", i):
            newline = text.find("\n", i + 2)
            i = len(text) if newline < 0 else newline
            continue
        if c == "/" and text.startswith("/*", i):
            end = text.find("*/", i + 2)
            if end < 0:
                raise ValueError("unterminated block comment")
            i = end + 2
            if current and current[-1] != " ":
                current.append(" ")
            continue
        if c in "\"'":
            end = _skip_quoted(text, i)
            current.append(text[i:end])
            seen = True
            i = end
            continue
        if c == "(":
            depth += 1
            current.append(c)
            seen = True
            i += 1
            continue
        if c == ")":
            depth -= 1
            if depth == 0:
                if seen:
                    finish()
                return args, i
            current.append(c)
            seen = True
            i += 1
            continue
        if c == "," and depth == 1:
            finish()
            current = []
            seen = False
            i += 1
            continue
        if c.isspace():
            if seen and current and current[-1] != " ":
                current.append(" ")
            i += 1
            continue
        current.append(c)
        seen = True
        i += 1
    raise ValueError("unterminated macro call")


def _find_calls(text: str) -> list[tuple[int, int, list[str], int]]:
    """Return (call start, open paren, args, close paren) for every call."""
    calls = []
    i = 0
    while i < len(text):
        c = text[i]
        if text.startswith("//", i):
            newline = text.find("\n", i + 2)
            i = len(text) if newline < 0 else newline + 1
            continue
        if text.startswith("/*", i):
            end = text.find("*/", i + 2)
            i = len(text) if end < 0 else end + 2
            continue
        if c in "\"'":
            i = _skip_quoted(text, i)
            continue
        line_start = text.rfind("\n", 0, i) + 1
        after_define = text[line_start:i].lstrip().startswith("#define")
        if (
            text.startswith(MACRO, i)
            and not after_define
            and (i == 0 or not (text[i - 1].isalnum() or text[i - 1] == "_"))
        ):
            j = i + len(MACRO)
            while j < len(text) and text[j].isspace():
                j += 1
            if j < len(text) and text[j] == "(":
                args, close = _parse_args(text, j)
                calls.append((i, j, args, close))
                i = close + 1
                continue
        i += 1
    return calls


def _line_indent(text: str, pos: int) -> str:
    line_start = text.rfind("\n", 0, pos) + 1
    indent = text[line_start:pos]
    return indent[: len(indent) - len(indent.lstrip(" \t"))]


def _render(args: list[str], indent: str) -> str:
    lines = []
    pos = 0
    for title, rows in GROUPS:
        lines.append(f"{indent}// {title}")
        for count in rows:
            chunk = args[pos : pos + count]
            pos += count
            line = indent + ", ".join(chunk)
            if pos != len(args):
                line += ","
            lines.append(line)
    return "\n" + "\n".join(lines)


def format_text(text: str, source: str) -> tuple[str, bool]:
    calls = _find_calls(text)
    if not calls:
        return text, False
    changes = False
    for start, open_paren, args, close in reversed(calls):
        if len(args) != ARG_COUNT:
            raise ValueError(
                f"{source}: {MACRO} call has {len(args)} arguments, expected {ARG_COUNT}"
            )
        indent = _line_indent(text, start) + "    "
        inner = _render(args, indent)
        if text[open_paren + 1 : close] != inner:
            changes = True
            text = text[: open_paren + 1] + inner + text[close:]
    return text, changes


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--check", action="store_true", help="exit 1 if a file would change")
    parser.add_argument("files", nargs="+", type=Path, help="C source files to format")
    opts = parser.parse_args()

    status = 0
    for path in opts.files:
        try:
            original = path.read_text(encoding="utf-8")
            formatted, changed = format_text(original, str(path))
        except (OSError, ValueError) as err:
            print(f"error: {err}", file=sys.stderr)
            status = 1
            continue
        if opts.check:
            if changed:
                print(f"{path}: needs formatting")
                status = 1
        elif changed:
            path.write_text(formatted, encoding="utf-8")
            print(f"formatted {path}")
    return status


if __name__ == "__main__":
    sys.exit(main())
