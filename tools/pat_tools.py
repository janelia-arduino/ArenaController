#!/usr/bin/env python3
from __future__ import annotations

import argparse
import re
import shutil
import sys
from pathlib import Path


def parse_numbered_filename(filename: str) -> tuple[str, str, str]:
    """
    Split a filename into (prefix, digits, suffix) where digits is the last run of digits
    before the extension.

    Examples:
      PAT0004.pat  -> ("PAT", "0004", ".pat")
      foo12.txt    -> ("foo", "12", ".txt")
      ABC001       -> ("ABC", "001", "")
    """
    m = re.match(r"^(?P<prefix>.*?)(?P<num>\d+)(?P<suffix>\.[^.]*)?$", filename)
    if not m:
        raise ValueError(
            f"Filename {filename!r} does not contain a numeric part (e.g. PAT0004.pat)."
        )
    prefix = m.group("prefix") or ""
    num = m.group("num")
    suffix = m.group("suffix") or ""
    return prefix, num, suffix


def cmd_copy_file(args: argparse.Namespace) -> int:
    src: Path = args.source
    copies: int = args.copies
    start: int = args.start
    overwrite: bool = args.overwrite
    dry_run: bool = args.dry_run
    quiet: bool = args.quiet

    if copies <= 0:
        print("error: copies must be > 0", file=sys.stderr)
        return 2
    if start < 0:
        print("error: start_number must be >= 0", file=sys.stderr)
        return 2

    if not src.exists():
        print(f"error: source file does not exist: {src}", file=sys.stderr)
        return 2
    if not src.is_file():
        print(f"error: source is not a file: {src}", file=sys.stderr)
        return 2

    directory = src.parent
    prefix, src_digits, suffix = parse_numbered_filename(src.name)

    # Determine zero-padding width:
    # - default to source width (e.g. 4 for PAT0004.pat)
    # - expand if needed to fit the largest destination number
    max_dest = start + copies - 1
    width = max(len(src_digits), len(str(max_dest)))
    if args.width is not None:
        if args.width <= 0:
            print("error: --width must be > 0", file=sys.stderr)
            return 2
        width = args.width

    # Build destination paths first, so we can abort safely if collisions occur.
    dest_paths: list[Path] = []
    for i in range(copies):
        n = start + i
        dest_name = f"{prefix}{n:0{width}d}{suffix}"
        dest_paths.append(directory / dest_name)

    # Prevent copying onto the source file itself.
    try:
        src_resolved = src.resolve()
        for dp in dest_paths:
            if dp.resolve() == src_resolved:
                print(
                    f"error: destination list includes the source file itself ({src.name}). "
                    f"Choose a different start_number.",
                    file=sys.stderr,
                )
                return 2
    except Exception:
        # If resolve() fails for some reason (rare), continue without this check.
        pass

    existing = [p for p in dest_paths if p.exists()]
    if existing and not overwrite:
        print(
            "error: one or more destination files already exist. "
            "Re-run with --overwrite to replace them, or choose a different start_number.\n"
            "Existing files:\n  "
            + "\n  ".join(str(p) for p in existing[:20])
            + ("" if len(existing) <= 20 else f"\n  ... ({len(existing) - 20} more)"),
            file=sys.stderr,
        )
        return 2

    created = 0
    for dp in dest_paths:
        if dry_run:
            if not quiet:
                print(f"DRY RUN: would copy -> {dp.name}")
            created += 1
            continue

        # copy2 preserves metadata when possible (mtime, etc.)
        shutil.copy2(src, dp)
        created += 1
        if not quiet:
            print(f"copied -> {dp.name}")

    if not quiet:
        print(f"Done. Created {created} file(s) in {directory}")
    return 0


def cmd_delete_files(args: argparse.Namespace) -> int:
    directory: Path = args.directory
    min_number: int = args.min_number
    prefix: str = args.prefix
    ext: str = args.ext
    dry_run: bool = args.dry_run
    quiet: bool = args.quiet

    if min_number < 0:
        print("error: min_number must be >= 0", file=sys.stderr)
        return 2
    if not ext.startswith(".") and ext != "":
        ext = "." + ext

    if not directory.exists():
        print(f"error: directory does not exist: {directory}", file=sys.stderr)
        return 2
    if not directory.is_dir():
        print(f"error: not a directory: {directory}", file=sys.stderr)
        return 2

    # Match files like PAT0100.pat (prefix + digits + ext), digits capture group.
    pattern = re.compile(rf"^{re.escape(prefix)}(?P<num>\d+){re.escape(ext)}$")

    deleted = 0
    candidates = 0

    for entry in directory.iterdir():
        if not entry.is_file():
            continue

        m = pattern.match(entry.name)
        if not m:
            continue

        candidates += 1
        n = int(m.group("num"))  # int("0100") -> 100
        if n >= min_number:
            if dry_run:
                if not quiet:
                    print(f"DRY RUN: would delete {entry.name}")
                deleted += 1
            else:
                entry.unlink()
                deleted += 1
                if not quiet:
                    print(f"deleted {entry.name}")

    if not quiet:
        action = "Would delete" if dry_run else "Deleted"
        print(
            f"{action} {deleted} file(s) in {directory} (scanned {candidates} matching file(s))."
        )
    return 0


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        prog="pat_tools.py",
        description="Copy numbered files like PAT0004.pat to sequential names, and delete numbered files >= a threshold.",
    )
    sub = p.add_subparsers(dest="command", required=True)

    p_copy = sub.add_parser(
        "copy-file",
        help="Copy a numbered file N times into its directory, starting at a given destination number.",
    )
    p_copy.add_argument(
        "source", type=Path, help="Path to source file, e.g. /path/to/PAT0004.pat"
    )
    p_copy.add_argument("copies", type=int, help="How many copies to make (e.g. 25)")
    p_copy.add_argument(
        "start", type=int, help="Starting destination number (e.g. 100 -> PAT0100.pat)"
    )
    p_copy.add_argument(
        "--overwrite",
        action="store_true",
        help="Overwrite destination files if they already exist (default: error if any exist).",
    )
    p_copy.add_argument(
        "--width",
        type=int,
        default=None,
        help="Force numeric zero-padding width (default: inferred from source / range).",
    )
    p_copy.add_argument(
        "--dry-run",
        action="store_true",
        help="Print what would happen without copying.",
    )
    p_copy.add_argument("--quiet", action="store_true", help="Only print errors.")
    p_copy.set_defaults(func=cmd_copy_file)

    p_del = sub.add_parser(
        "delete-files",
        help="Delete files matching PREFIX + digits + EXT where digits >= min_number.",
    )
    p_del.add_argument(
        "directory", type=Path, help="Directory to clean, e.g. /path/to/patterns/"
    )
    p_del.add_argument(
        "min_number",
        type=int,
        help="Delete files with number >= this value (e.g. 100 deletes PAT0100.pat and above).",
    )
    p_del.add_argument(
        "--prefix", default="PAT", help="Filename prefix to match (default: PAT)"
    )
    p_del.add_argument(
        "--ext", default=".pat", help="Extension to match (default: .pat)"
    )
    p_del.add_argument(
        "--dry-run",
        action="store_true",
        help="Print what would be deleted without deleting.",
    )
    p_del.add_argument("--quiet", action="store_true", help="Only print errors.")
    p_del.set_defaults(func=cmd_delete_files)

    return p


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    raise SystemExit(main())
