#!/usr/bin/env python3
"""
Synchronize the project version across:

- README.org          (#+PROPERTY: VERSION X.Y.Z and '- Version: X.Y.Z')
- library.properties  (version=X.Y.Z)
- pixi.toml           ([workspace] version = "X.Y.Z")

Usage:
  python tools/version_sync.py check
  python tools/version_sync.py set 6.0.1

Tip:
  With the pixi.toml in this repo you can run:
    pixi run check-version
    pixi run set-version 6.0.1
"""

from __future__ import annotations

import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
README = ROOT / "README.org"
LIBPROPS = ROOT / "library.properties"
PIXI = ROOT / "pixi.toml"

# Accept: 1.2.3, 1.2.3-rc1, 1.2.3+build.7, 1.2.3-rc1+build.7
SEMVER_RE = re.compile(r"^\d+\.\d+\.\d+([\-+][0-9A-Za-z\.\-]+)?$")


def _read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="strict")


def _write_text(path: Path, text: str) -> None:
    path.write_text(text, encoding="utf-8", newline="\n")


def _replace_exactly_one(path: Path, pattern: str, repl: str) -> None:
    text = _read_text(path)
    new_text, n = re.subn(pattern, repl, text, flags=re.MULTILINE)
    if n != 1:
        raise RuntimeError(
            f"{path}: expected exactly 1 match for pattern, found {n}\npattern={pattern}"
        )
    if new_text != text:
        _write_text(path, new_text)


def _get_readme_version() -> str | None:
    if not README.exists():
        return None
    m = re.search(r"^#\+PROPERTY:\s+VERSION\s+(.+?)\s*$", _read_text(README), re.MULTILINE)
    return m.group(1).strip() if m else None


def _get_library_properties_version() -> str | None:
    if not LIBPROPS.exists():
        return None
    m = re.search(r"^version=(.+?)\s*$", _read_text(LIBPROPS), re.MULTILINE)
    return m.group(1).strip() if m else None


def _get_pixi_version() -> str | None:
    if not PIXI.exists():
        return None
    text = _read_text(PIXI)
    in_workspace = False
    for line in text.splitlines():
        if re.match(r"^\s*\[workspace\]\s*$", line):
            in_workspace = True
            continue
        if in_workspace and re.match(r"^\s*\[.+\]\s*$", line):
            # next section
            in_workspace = False
        if in_workspace:
            m = re.match(r'^\s*version\s*=\s*"([^"]+)"\s*$', line)
            if m:
                return m.group(1).strip()
    return None


def check() -> int:
    versions = {
        "README.org": _get_readme_version(),
        "library.properties": _get_library_properties_version(),
        "pixi.toml": _get_pixi_version(),
    }

    present = {k: v for k, v in versions.items() if v is not None}
    if not present:
        print(
            "No versions found.\n"
            "Expected:\n"
            "  - README.org contains '#+PROPERTY: VERSION ...'\n"
            "  - library.properties contains 'version=...'\n"
            "  - pixi.toml contains '[workspace] version = \"...\"'\n"
        )
        return 2

    unique = sorted(set(present.values()))
    if len(unique) != 1:
        print("Version mismatch:")
        for k, v in versions.items():
            print(f"  {k:18} {v}")
        return 1

    print(f"OK: version = {unique[0]}")
    return 0


def set_version(new_version: str) -> int:
    if not SEMVER_RE.match(new_version):
        print(
            f"ERROR: '{new_version}' does not look like a semver (examples: 6.0.1, 6.0.1-rc1).",
            file=sys.stderr,
        )
        return 2

    # README.org
    if README.exists():
        text = _read_text(README)
        if re.search(r"^#\+PROPERTY:\s+VERSION\s+.+$", text, re.MULTILINE):
            _replace_exactly_one(
                README,
                r"^#\+PROPERTY:\s+VERSION\s+.+$",
                f"#+PROPERTY: VERSION {new_version}",
            )
        else:
            # Insert VERSION property near the top
            lines = text.splitlines()
            insert_at = 1 if lines and lines[0].startswith("#+TITLE:") else 0
            lines.insert(insert_at, f"#+PROPERTY: VERSION {new_version}")
            _write_text(README, "\n".join(lines) + "\n")

        # Also keep the human-readable repo info line in sync (if present)
        # (We only update the first match to keep edits deterministic.)
        text2 = _read_text(README)
        text2, n = re.subn(
            r"^- Version:\s+.+$",
            f"- Version: {new_version}",
            text2,
            count=1,
            flags=re.MULTILINE,
        )
        if n == 1:
            _write_text(README, text2)

    # library.properties
    if LIBPROPS.exists():
        _replace_exactly_one(LIBPROPS, r"^version=.+$", f"version={new_version}")

    # pixi.toml ([workspace] version = "...")
    if PIXI.exists():
        text = _read_text(PIXI).splitlines()
        out_lines: list[str] = []
        in_workspace = False
        replaced = False
        for line in text:
            if re.match(r"^\s*\[workspace\]\s*$", line):
                in_workspace = True
                out_lines.append(line)
                continue
            if in_workspace and re.match(r"^\s*\[.+\]\s*$", line):
                in_workspace = False
            if in_workspace and re.match(r'^\s*version\s*=\s*".*"\s*$', line):
                out_lines.append(f'version = "{new_version}"')
                replaced = True
            else:
                out_lines.append(line)
        if not replaced:
            raise RuntimeError(f"{PIXI}: could not find [workspace] version = \"...\" line to update.")
        _write_text(PIXI, "\n".join(out_lines) + "\n")

    print(f"Updated version to {new_version}")
    return 0


def main(argv: list[str]) -> int:
    if len(argv) < 2:
        print(__doc__.strip())
        return 2

    cmd = argv[1]
    if cmd == "check":
        return check()
    if cmd == "set" and len(argv) == 3:
        return set_version(argv[2])

    print(__doc__.strip())
    return 2


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
