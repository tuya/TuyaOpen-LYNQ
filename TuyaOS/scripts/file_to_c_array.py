#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Convert a file to a C array definition.
Usage:
  python file_to_c_array.py <file> [options]
  python file_to_c_array.py audio.pcm -n audio_test_pcm_buffer -o audio_data.c
"""

import argparse
import os
import re
import sys


def sanitize_name(name: str) -> str:
    """Build a valid C identifier from filename (no path, no extension)."""
    base = os.path.splitext(os.path.basename(name))[0]
    base = re.sub(r"[^a-zA-Z0-9_]", "_", base)
    if base and base[0].isdigit():
        base = "data_" + base
    return base if base else "data"


def file_to_c_array(
    filepath: str,
    array_name: str,
    bytes_per_line: int = 16,
    add_size_symbol: bool = True,
) -> str:
    """Read file as binary and return C array source."""
    with open(filepath, "rb") as f:
        data = f.read()
    size = len(data)
    lines = []
    for i in range(0, size, bytes_per_line):
        chunk = data[i : i + bytes_per_line]
        hex_str = ", ".join("0x{:02X}".format(b) for b in chunk)
        lines.append("    " + hex_str + ",")
    body = "\n".join(lines)
    if add_size_symbol:
        out = (
            f"/* Generated from {os.path.basename(filepath)} ({size} bytes) */\n"
            f"static const unsigned char {array_name}[] = {{\n"
            f"{body}\n"
            f"}};\n"
            f"#define {array_name.upper()}_SIZE ({size})\n"
        )
    else:
        out = (
            f"/* Generated from {os.path.basename(filepath)} ({size} bytes) */\n"
            f"static const unsigned char {array_name}[] = {{\n"
            f"{body}\n"
            f"}};\n"
        )
    return out


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Convert a file to a C array definition.",
        epilog="Example: %(prog)s audio.pcm -n audio_test_pcm_buffer -o audio_data.c",
    )
    parser.add_argument(
        "file",
        metavar="FILE",
        help="Input file path (binary or text)",
    )
    parser.add_argument(
        "-n",
        "--name",
        dest="array_name",
        default=None,
        help="C array name (default: from filename, e.g. audio_pcm)",
    )
    parser.add_argument(
        "-o",
        "--output",
        dest="output",
        default=None,
        help="Output .c or .h file (default: print to stdout)",
    )
    parser.add_argument(
        "-b",
        "--bytes-per-line",
        type=int,
        default=16,
        metavar="N",
        help="Bytes per line in output (default: 16)",
    )
    parser.add_argument(
        "--no-size-macro",
        action="store_true",
        help="Do not add ARRAY_NAME_SIZE macro",
    )
    args = parser.parse_args()

    if not os.path.isfile(args.file):
        print(f"Error: file not found: {args.file}", file=sys.stderr)
        return 1

    array_name = args.array_name or sanitize_name(args.file)
    if not re.match(r"^[a-zA-Z_][a-zA-Z0-9_]*$", array_name):
        print(f"Error: invalid array name '{array_name}'", file=sys.stderr)
        return 1

    try:
        result = file_to_c_array(
            args.file,
            array_name,
            bytes_per_line=args.bytes_per_line,
            add_size_symbol=not args.no_size_macro,
        )
    except OSError as e:
        print(f"Error reading file: {e}", file=sys.stderr)
        return 1

    if args.output:
        with open(args.output, "w", encoding="utf-8") as f:
            f.write(result)
        print(f"Written {len(result)} chars to {args.output}", file=sys.stderr)
    else:
        print(result)

    return 0


if __name__ == "__main__":
    sys.exit(main())
