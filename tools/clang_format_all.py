#!/usr/bin/env python3
"""
Format all tracked C/C++/Arduino files using clang-format.

Usage:
  python tools/clang_format_all.py         # apply formatting in-place
  python tools/clang_format_all.py --check # check only (no changes), exit nonzero if not formatted
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
from pathlib import Path
from shutil import which


EXTS = (".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".ino")


def repo_root() -> Path:
    # Prefer Pixi-provided root when available
    env_root = os.environ.get("PIXI_PROJECT_ROOT")
    if env_root:
        return Path(env_root).resolve()
    # Fallback: tools/ -> repo root
    return Path(__file__).resolve().parents[1]


def git_tracked_sources(root: Path) -> list[Path]:
    # Use git so we only format tracked source files (avoids .pio/.pixi/.tools, etc.)
    # Works cross-platform; no shell glob expansion needed.
    patterns = [f"*{ext}" for ext in EXTS]
    cmd = ["git", "-C", str(root), "ls-files", "-z", "--"] + patterns
    res = subprocess.run(
        cmd, check=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    if res.returncode != 0:
        sys.stderr.write("ERROR: git ls-files failed:\n")
        sys.stderr.write(res.stderr.decode(errors="replace"))
        return []
    raw = res.stdout.split(b"\0")
    files = [Path(root, p.decode("utf-8")) for p in raw if p]
    return files


def run_clang_format(files: list[Path], check_only: bool) -> int:
    clang_format = which("clang-format") or which("clang-format.exe")
    if not clang_format:
        sys.stderr.write(
            "ERROR: clang-format not found on PATH. Add clang-tools/clang-format to pixi dependencies.\n"
        )
        return 2

    # For check-only we use --dry-run + --Werror (clang-format supports both). :contentReference[oaicite:10]{index=10}
    base = [clang_format, "--style=file"]
    if check_only:
        base += ["--dry-run", "--Werror"]
    else:
        base += ["-i"]

    # Run file-by-file to avoid command line length limits on Windows.
    rc = 0
    for f in files:
        r = subprocess.run(base + [str(f)], check=False)
        if r.returncode != 0:
            rc = r.returncode or 1
    return rc


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--check",
        action="store_true",
        help="Check formatting only; do not modify files.",
    )
    args = ap.parse_args()

    root = repo_root()
    files = git_tracked_sources(root)

    if not files:
        print("No tracked C/C++/Arduino files found (or git not available).")
        return 0

    return run_clang_format(files, check_only=args.check)


if __name__ == "__main__":
    raise SystemExit(main())
