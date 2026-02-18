#!/usr/bin/env python3
"""
quantum_leaps_tools.py

Install and run the Quantum Leaps host tools that are pinned to match the
qp-arduino (qpcpp) version used by this repository:

- QM      5.2.3
- QTools  6.9.3  (includes QSPY)

The tools are installed locally into:
  <repo>/.tools/quantum-leaps/

This keeps the repository buildable on Linux/macOS/Windows via `pixi`, without
requiring system-wide installs.

Typical usage (via pixi tasks):

  # Install both QM and QTools
  pixi run ql-install

  # Launch QM
  pixi run qm

  # Run QSPY (arguments are forwarded to qspy)
  pixi run qspy -c /dev/ttyACM0 -b 115200
"""

from __future__ import annotations

import argparse
import os
import platform
import shutil
import subprocess
import sys
import tempfile
import textwrap
import urllib.error
import urllib.request
import zipfile
from pathlib import Path
from typing import Iterable, Optional

QM_VERSION = "5.2.3"
QTOOLS_VERSION = "6.9.3"


def _repo_root() -> Path:
    # This file is intended to live at <repo>/tools/quantum_leaps_tools.py
    return Path(__file__).resolve().parents[1]


def _tools_base_dir() -> Path:
    # Allow override if a user doesn't want installs inside the repo.
    # Example:
    #   QL_TOOLS_DIR=~/.cache/arena-controller/quantum-leaps pixi run ql-install
    override = os.environ.get("QL_TOOLS_DIR")
    if override:
        return Path(override).expanduser().resolve()
    return _repo_root() / ".tools" / "quantum-leaps"


def _cache_dir() -> Path:
    return _tools_base_dir() / "_cache"


def _sys_id() -> tuple[str, str]:
    sysname = platform.system().lower()  # linux / windows / darwin
    machine = platform.machine().lower()
    return sysname, machine


def _print(msg: str) -> None:
    print(msg, flush=True)


def _eprint(msg: str) -> None:
    print(msg, file=sys.stderr, flush=True)


def _is_probably_html(path: Path) -> bool:
    try:
        head = path.read_bytes()[:512].lstrip()
    except Exception:
        return False
    return head.startswith(b"<!doctype html") or head.startswith(b"<html") or head.startswith(b"<HTML")


def _download(urls: Iterable[str], dest: Path, *, force: bool = False, validate_zip: bool = False) -> Path:
    """
    Download from the first working URL into dest.

    validate_zip=True performs zipfile.is_zipfile() validation to avoid saving
    HTML redirect pages into a .zip file.
    """
    dest.parent.mkdir(parents=True, exist_ok=True)

    if dest.exists() and not force:
        return dest

    tmp = dest.with_suffix(dest.suffix + ".tmp")
    if tmp.exists():
        tmp.unlink()

    last_err: Optional[BaseException] = None

    for url in urls:
        try:
            _print(f"Downloading: {url}")
            req = urllib.request.Request(
                url,
                headers={
                    # Some hosts behave differently depending on UA.
                    "User-Agent": "ArenaController/quantum_leaps_tools.py (+https://github.com/janelia-arduino/ArenaController)",
                },
            )
            with urllib.request.urlopen(req) as r, open(tmp, "wb") as f:
                shutil.copyfileobj(r, f)

            if validate_zip:
                if not zipfile.is_zipfile(tmp) or _is_probably_html(tmp):
                    raise RuntimeError("downloaded file is not a valid zip archive (maybe an HTML redirect page)")

            tmp.replace(dest)
            return dest

        except (urllib.error.HTTPError, urllib.error.URLError, RuntimeError, OSError) as e:
            last_err = e
            _eprint(f"  failed: {e}")
            if tmp.exists():
                tmp.unlink(missing_ok=True)  # type: ignore[attr-defined]
            continue

    raise SystemExit(f"ERROR: Unable to download {dest.name}. Last error: {last_err}")


def _extract_zip_move_top(
    archive: Path,
    dest_dir: Path,
    *,
    expected_top_dir_name: Optional[str] = None,
    force: bool = False,
) -> Path:
    """
    Extract a zip archive and move a top-level directory into dest_dir.

    Many Quantum Leaps archives contain a single directory at the zip root
    (e.g., 'qm/' or 'qtools/'). We standardize installs into versioned folders.
    """
    if dest_dir.exists() and any(dest_dir.iterdir()) and not force:
        return dest_dir

    if dest_dir.exists() and force:
        shutil.rmtree(dest_dir)

    dest_dir.parent.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory(prefix="ql_zip_") as td:
        tmp_root = Path(td)
        with zipfile.ZipFile(archive, "r") as z:
            z.extractall(tmp_root)

        src_dir: Optional[Path] = None

        if expected_top_dir_name:
            for p in tmp_root.iterdir():
                if p.is_dir() and p.name.lower() == expected_top_dir_name.lower():
                    src_dir = p
                    break

        if src_dir is None:
            dirs = [p for p in tmp_root.iterdir() if p.is_dir()]
            files = [p for p in tmp_root.iterdir() if p.is_file()]
            if len(dirs) == 1 and not files:
                src_dir = dirs[0]

        if src_dir is None:
            # Fall back: copy whole extracted tree into dest_dir.
            dest_dir.mkdir(parents=True, exist_ok=True)
            for item in tmp_root.iterdir():
                dst = dest_dir / item.name
                if item.is_dir():
                    shutil.copytree(item, dst, dirs_exist_ok=True)
                else:
                    shutil.copy2(item, dst)
            return dest_dir

        shutil.move(str(src_dir), str(dest_dir))

    return dest_dir


def _qm_candidate_urls(sysname: str, machine: str) -> list[str]:
    tag = f"v{QM_VERSION}"
    gh_base = f"https://github.com/QuantumLeaps/qm/releases/download/{tag}/"

    urls: list[str] = []

    if sysname == "linux":
        # Known-good for qp-arduino toolchain documentation:
        urls.append(gh_base + f"qm_{QM_VERSION}-linux64.zip")
        # Some newer releases switched naming (keep as fallback):
        urls.append(gh_base + f"qm_{QM_VERSION}-linux.zip")

    elif sysname == "windows":
        # Try GitHub first
        urls.extend(
            [
                gh_base + f"qm_{QM_VERSION}-windows.zip",
                gh_base + f"qm_{QM_VERSION}-win64.zip",
                gh_base + f"qm_{QM_VERSION}-win32.zip",
            ]
        )
        # SourceForge fallbacks (older releases sometimes live here)
        urls.extend(
            [
                f"https://sourceforge.net/projects/qpc/files/QM/{QM_VERSION}/qm_{QM_VERSION}-windows.zip/download",
                f"https://sourceforge.net/projects/qpc/files/QM/{QM_VERSION}/qm_{QM_VERSION}-win64.zip/download",
                f"https://sourceforge.net/projects/qpc/files/QM/{QM_VERSION}/qm_{QM_VERSION}-win32.zip/download",
            ]
        )

    elif sysname == "darwin":
        # Historical naming in qp-arduino docs uses macx64 for older QM versions.
        urls.extend(
            [
                gh_base + f"qm_{QM_VERSION}-macx64.dmg",
                gh_base + f"qm_{QM_VERSION}-macos.dmg",
                gh_base + f"qm_{QM_VERSION}-macosx.dmg",
                f"https://sourceforge.net/projects/qpc/files/QM/{QM_VERSION}/qm_{QM_VERSION}-macx64.dmg/download",
                f"https://sourceforge.net/projects/qpc/files/QM/{QM_VERSION}/qm_{QM_VERSION}-macos.dmg/download",
            ]
        )
    else:
        raise SystemExit(f"Unsupported OS: {sysname}")

    return urls


def _qtools_candidate_urls(sysname: str) -> list[str]:
    tag = f"v{QTOOLS_VERSION}"
    gh_base = f"https://github.com/QuantumLeaps/qtools/releases/download/{tag}/"

    if sysname == "windows":
        fname = f"qtools-windows_{QTOOLS_VERSION}.zip"
    else:
        fname = f"qtools-posix_{QTOOLS_VERSION}.zip"

    return [
        gh_base + fname,
        f"https://sourceforge.net/projects/qtools/files/v{QTOOLS_VERSION}/{fname}/download",
    ]


def install_qm(*, force: bool = False) -> Path:
    sysname, machine = _sys_id()
    base = _tools_base_dir()
    cache = _cache_dir()
    cache.mkdir(parents=True, exist_ok=True)

    install_dir = base / f"qm-{QM_VERSION}"

    if sysname == "darwin":
        dmg = cache / f"qm_{QM_VERSION}.dmg"
        _download(_qm_candidate_urls(sysname, machine), dmg, force=force, validate_zip=False)

        if install_dir.exists() and any(install_dir.iterdir()) and not force:
            return install_dir

        if install_dir.exists() and force:
            shutil.rmtree(install_dir)

        install_dir.mkdir(parents=True, exist_ok=True)

        # Try to mount the DMG and copy the .app bundle into install_dir.
        # This avoids requiring admin permissions for /Applications.
        mount_point = Path(tempfile.mkdtemp(prefix="qm_dmg_mount_"))
        try:
            # Clear quarantine bit on the dmg (best-effort)
            subprocess.run(["xattr", "-c", str(dmg)], check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

            subprocess.run(
                ["hdiutil", "attach", "-nobrowse", "-readonly", "-mountpoint", str(mount_point), str(dmg)],
                check=True,
            )

            apps = list(mount_point.glob("*.app"))
            if not apps:
                apps = list(mount_point.rglob("*.app"))
            if not apps:
                raise SystemExit(f"ERROR: Could not find a *.app inside mounted DMG at {mount_point}")

            app_src = apps[0]
            app_dst = install_dir / app_src.name
            if app_dst.exists():
                shutil.rmtree(app_dst)
            shutil.copytree(app_src, app_dst, dirs_exist_ok=True)

            # Clear quarantine on the copied app (best-effort)
            subprocess.run(["xattr", "-cr", str(app_dst)], check=False)

            return install_dir

        finally:
            subprocess.run(["hdiutil", "detach", str(mount_point)], check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            shutil.rmtree(mount_point, ignore_errors=True)

    else:
        zip_path = cache / f"qm_{QM_VERSION}_{sysname}.zip"
        _download(_qm_candidate_urls(sysname, machine), zip_path, force=force, validate_zip=True)
        _extract_zip_move_top(zip_path, install_dir, expected_top_dir_name="qm", force=force)

        # Ensure Linux executable bits
        if sysname == "linux":
            for exe in (install_dir / "bin" / "qm", install_dir / "bin" / "qm.sh"):
                if exe.exists():
                    exe.chmod(exe.stat().st_mode | 0o111)

        return install_dir


def _find_qspy_binary(qtools_dir: Path) -> Optional[Path]:
    sysname, _ = _sys_id()
    if sysname == "windows":
        candidates = [
            qtools_dir / "bin" / "qspy.exe",
            qtools_dir / "qspy" / "bin" / "qspy.exe",
            qtools_dir / "qspy.exe",
        ]
        name = "qspy.exe"
    else:
        candidates = [
            qtools_dir / "bin" / "qspy",
            qtools_dir / "qspy" / "bin" / "qspy",
            qtools_dir / "qspy",
        ]
        name = "qspy"

    for p in candidates:
        if p.exists():
            return p

    # Fallback: search (can be slower, but tools dirs are not huge)
    matches = list(qtools_dir.rglob(name))
    return matches[0] if matches else None


def install_qtools(*, force: bool = False, build_qspy: bool = True) -> Path:
    sysname, _ = _sys_id()
    base = _tools_base_dir()
    cache = _cache_dir()
    cache.mkdir(parents=True, exist_ok=True)

    if sysname == "windows":
        fname = f"qtools-windows_{QTOOLS_VERSION}.zip"
    else:
        fname = f"qtools-posix_{QTOOLS_VERSION}.zip"

    zip_path = cache / fname
    install_dir = base / f"qtools-{QTOOLS_VERSION}"

    _download(_qtools_candidate_urls(sysname), zip_path, force=force, validate_zip=True)
    _extract_zip_move_top(zip_path, install_dir, expected_top_dir_name="qtools", force=force)

    # Try to build qspy on POSIX if it's not present.
    if sysname != "windows" and build_qspy:
        qspy_bin = _find_qspy_binary(install_dir)
        if qspy_bin is None:
            posix_dir = install_dir / "qspy" / "posix"
            if posix_dir.exists():
                _print("Building qspy from source (make) ...")
                try:
                    subprocess.run(["make"], cwd=str(posix_dir), check=True)
                except FileNotFoundError:
                    _eprint("ERROR: 'make' not found. Install build tools and re-run: pixi run qtools-install")
                except subprocess.CalledProcessError as e:
                    _eprint(f"ERROR: qspy build failed: {e}")
            else:
                _eprint(f"WARNING: expected directory not found: {posix_dir}")

    return install_dir


def run_qm() -> None:
    sysname, _ = _sys_id()
    qm_dir = install_qm(force=False)

    if sysname == "linux":
        entry = qm_dir / "bin" / "qm.sh"
        if not entry.exists():
            entry = qm_dir / "bin" / "qm"
        if not entry.exists():
            raise SystemExit(f"ERROR: QM executable not found under: {qm_dir}/bin")
        entry.chmod(entry.stat().st_mode | 0o111)
        subprocess.run([str(entry)], check=False)

    elif sysname == "windows":
        candidates = [
            qm_dir / "bin" / "qm.exe",
            qm_dir / "qm.exe",
            qm_dir / "bin" / "qm.bat",
            qm_dir / "qm.bat",
        ]
        entry = next((p for p in candidates if p.exists()), None)
        if entry is None:
            # Fallback search
            matches = list(qm_dir.rglob("qm.exe")) + list(qm_dir.rglob("qm.bat"))
            entry = matches[0] if matches else None
        if entry is None:
            raise SystemExit(f"ERROR: QM executable not found under: {qm_dir}")
        subprocess.run([str(entry)], check=False)

    elif sysname == "darwin":
        apps = list(qm_dir.glob("*.app"))
        if not apps:
            apps = list(qm_dir.rglob("*.app"))
        if not apps:
            raise SystemExit(f"ERROR: QM app bundle (*.app) not found under: {qm_dir}")
        app = apps[0]
        subprocess.run(["open", "-a", str(app)], check=False)

    else:
        raise SystemExit(f"Unsupported OS: {sysname}")


def run_qspy(qspy_args: list[str]) -> int:
    sysname, _ = _sys_id()
    qtools_dir = install_qtools(force=False, build_qspy=True)
    qspy = _find_qspy_binary(qtools_dir)
    if qspy is None:
        raise SystemExit(
            "ERROR: qspy binary was not found after installing QTools.\n\n"
            "On Linux/macOS you may need build tools (make + a C compiler).\n"
            "Try:\n"
            "  pixi run qtools-install\n"
        )

    if sysname != "windows":
        qspy.chmod(qspy.stat().st_mode | 0o111)

    # Some QTools utilities look for QTOOLS/QTOOLS_HOME.
    env = os.environ.copy()
    env.setdefault("QTOOLS", str(qtools_dir))
    env.setdefault("QTOOLS_HOME", str(qtools_dir))

    cmd = [str(qspy)] + qspy_args
    _print("Running: " + " ".join(cmd))
    proc = subprocess.run(cmd, env=env, check=False)
    return proc.returncode


def install_all(*, force: bool = False) -> None:
    install_qm(force=force)
    install_qtools(force=force, build_qspy=True)
    _print(f"Installed QM {QM_VERSION} and QTools {QTOOLS_VERSION} into: {_tools_base_dir()}")


def main(argv: Optional[list[str]] = None) -> int:
    parser = argparse.ArgumentParser(
        prog="quantum_leaps_tools.py",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=textwrap.dedent(
            f"""\
            Install and run Quantum Leaps host tools pinned for this repository:

              QM      {QM_VERSION}
              QTools   {QTOOLS_VERSION} (includes QSPY)

            Tools are installed locally into:
              {str(_tools_base_dir())}

            Examples:
              python tools/quantum_leaps_tools.py install
              python tools/quantum_leaps_tools.py qm
              python tools/quantum_leaps_tools.py qspy -c /dev/ttyACM0 -b 115200
            """
        ),
    )

    sub = parser.add_subparsers(dest="command", required=True)

    p_install = sub.add_parser("install", help="Install both QM and QTools")
    p_install.add_argument("--force", action="store_true", help="Re-download and reinstall from scratch")

    p_qm = sub.add_parser("qm", help="Launch QM (installs first if needed)")
    p_qm.add_argument("--install-only", action="store_true", help="Only install QM (do not launch)")
    p_qm.add_argument("--force", action="store_true", help="Re-download and reinstall QM from scratch")

    p_qtools = sub.add_parser("qtools-install", help="Install QTools (and build qspy on POSIX if needed)")
    p_qtools.add_argument("--force", action="store_true", help="Re-download and reinstall QTools from scratch")

    p_qspy = sub.add_parser("qspy", help="Run QSPY (installs QTools first if needed)")
    p_qspy.add_argument("--force", action="store_true", help="Re-download and reinstall QTools from scratch")
    # Forward everything after 'qspy' to the qspy binary, verbatim.
    p_qspy.add_argument("qspy_args", nargs=argparse.REMAINDER, help="Arguments forwarded to qspy")

    args = parser.parse_args(argv)

    if args.command == "install":
        install_all(force=args.force)
        return 0

    if args.command == "qm":
        install_qm(force=args.force)
        if not args.install_only:
            run_qm()
        return 0

    if args.command == "qtools-install":
        install_qtools(force=args.force, build_qspy=True)
        return 0

    if args.command == "qspy":
        if args.force:
            install_qtools(force=True, build_qspy=True)
        qspy_args = list(args.qspy_args)
        # Pixi users sometimes use '--' separators; tolerate it.
        if qspy_args and qspy_args[0] == "--":
            qspy_args = qspy_args[1:]
        return run_qspy(qspy_args)

    raise SystemExit("unreachable")


if __name__ == "__main__":
    raise SystemExit(main())
