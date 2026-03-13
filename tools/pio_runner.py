#!/usr/bin/env python3
"""Cross-platform wrapper for PlatformIO example builds.

Sets PLATFORMIO_SRC_DIR and PLATFORMIO_BUILD_DIR in-process, then invokes
`pio run ...` with the requested environment/target. It also implements a
cross-platform `deepclean` action used by pixi.toml. This avoids shell-specific
`export ... && ...` and `rm -rf ...` task bodies.
"""
from __future__ import annotations

import argparse
import os
import shutil
import subprocess
from pathlib import Path


def project_root() -> Path:
    return Path(__file__).resolve().parent.parent


def build_env(example: str) -> dict[str, str]:
    root = project_root()
    env = os.environ.copy()
    env['PLATFORMIO_SRC_DIR'] = str(root / example)
    env['PLATFORMIO_BUILD_DIR'] = str(root / '.pio' / 'build' / example)
    return env


def remove_tree(path: Path) -> None:
    if path.exists():
        print(f"[pio_runner] removing {path}")
        shutil.rmtree(path)
    else:
        print(f"[pio_runner] already absent: {path}")


def deepclean(*, example: str) -> int:
    root = project_root()
    remove_tree(root / '.pio' / 'build' / example)
    remove_tree(root / '.pio' / 'libdeps')
    return 0


def run_pio(args: list[str], *, example: str) -> int:
    env = build_env(example)
    print(f"[pio_runner] PLATFORMIO_SRC_DIR={env['PLATFORMIO_SRC_DIR']}")
    print(f"[pio_runner] PLATFORMIO_BUILD_DIR={env['PLATFORMIO_BUILD_DIR']}")
    print(f"[pio_runner] exec: {' '.join(args)}")
    proc = subprocess.run(args, env=env)
    return proc.returncode


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument('action', choices=['build', 'clean', 'upload', 'flash', 'rebuild', 'deepclean'])
    parser.add_argument('--env', dest='env_name', required=True)
    parser.add_argument('--example', default='examples/ArenaControllerTeensy12-12')
    parser.add_argument('--port', default=None)
    ns = parser.parse_args()

    if ns.action == 'deepclean':
        return deepclean(example=ns.example)

    base = ['pio', 'run', '-e', ns.env_name]
    if ns.action == 'build':
        return run_pio(base, example=ns.example)
    if ns.action == 'clean':
        return run_pio(base + ['-t', 'clean'], example=ns.example)
    if ns.action == 'upload':
        cmd = base + ['-t', 'upload']
        if ns.port:
            cmd += ['--upload-port', ns.port]
        return run_pio(cmd, example=ns.example)
    if ns.action == 'flash':
        rc = run_pio(base + ['-t', 'clean'], example=ns.example)
        if rc != 0:
            return rc
        cmd = base + ['-t', 'upload']
        if ns.port:
            cmd += ['--upload-port', ns.port]
        return run_pio(cmd, example=ns.example)
    if ns.action == 'rebuild':
        rc = run_pio(base + ['-t', 'clean'], example=ns.example)
        if rc != 0:
            return rc
        return run_pio(base, example=ns.example)
    parser.error('unsupported action')
    return 2


if __name__ == '__main__':
    raise SystemExit(main())
