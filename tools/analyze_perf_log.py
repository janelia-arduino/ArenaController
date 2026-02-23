#!/usr/bin/env python3
"""Analyze ArenaController QS performance probe logs.

This script is designed to be robust to slightly different QSpy text formats.
It looks for lines containing these QS user record names:

  - PERF_FRAME
  - PERF_STAGE
  - PERF_DROP

and then extracts the numeric fields that follow.

Expected payloads (as emitted by the instrumentation added in fsp.cpp):

PERF_FRAME:
  START record fields:
    [0] type=0
    [1] frame_id
    [2] start_us
    [3] ifi_us
    [4] period_us

  END record fields:
    [0] type=1
    [1] frame_id
    [2] end_us
    [3] dur_us
    [4] flags

PERF_STAGE:
  [0] stage_id
  [1] start_us
  [2] dur_us
  [3] aux

PERF_DROP:
  varies (see instrumentation)

Usage:
  python analyze_perf_log.py path/to/qspy_output.txt

Tip:
  Run qspy with dictionaries enabled so the record names appear in the output.
"""

from __future__ import annotations

import argparse
import math
import re
from dataclasses import dataclass
from pathlib import Path
from statistics import mean
from typing import Dict, List, Optional, Tuple


NUM_RE = re.compile(r"0x[0-9a-fA-F]+|\b\d+\b")


def parse_int(tok: str) -> int:
    tok = tok.strip()
    if tok.startswith("0x") or tok.startswith("0X"):
        return int(tok, 16)
    return int(tok)


def percentiles(values: List[float], ps: List[float]) -> Dict[float, float]:
    if not values:
        return {p: math.nan for p in ps}
    xs = sorted(values)
    n = len(xs)

    def q(p: float) -> float:
        if n == 1:
            return float(xs[0])
        # Linear interpolation between closest ranks
        pos = (p / 100.0) * (n - 1)
        lo = int(math.floor(pos))
        hi = int(math.ceil(pos))
        if lo == hi:
            return float(xs[lo])
        frac = pos - lo
        return float(xs[lo]) * (1.0 - frac) + float(xs[hi]) * frac

    return {p: q(p) for p in ps}


def summarize_us(values_us: List[int], label: str) -> str:
    if not values_us:
        return f"{label}: (no samples)"
    vals = [float(v) for v in values_us]
    mu = mean(vals)
    # population std
    var = mean([(v - mu) ** 2 for v in vals])
    sd = math.sqrt(var)
    p = percentiles(vals, [50, 90, 99, 99.9])
    return (
        f"{label}: n={len(values_us)} min={min(values_us)}us p50={p[50]:.1f}us "
        f"p90={p[90]:.1f}us p99={p[99]:.1f}us p99.9={p[99.9]:.1f}us "
        f"mean={mu:.1f}us std={sd:.1f}us max={max(values_us)}us"
    )


@dataclass
class FrameStart:
    frame_id: int
    start_us: int
    ifi_us: int
    period_us: int


@dataclass
class FrameEnd:
    frame_id: int
    end_us: int
    dur_us: int
    flags: int


@dataclass
class Stage:
    stage_id: int
    start_us: int
    dur_us: int
    aux: int


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("log", type=Path, help="QSpy text output file")
    args = ap.parse_args()

    starts: List[FrameStart] = []
    ends: List[FrameEnd] = []
    stages: List[Stage] = []
    drops_raw: List[Tuple[str, List[int]]] = []

    with args.log.open("r", errors="ignore") as f:
        for line in f:
            if "PERF_FRAME" in line:
                toks = [parse_int(t) for t in NUM_RE.findall(line)]
                # Heuristic: look for the first 5 ints after the record name.
                # If the line contains other numbers (timestamps, priorities),
                # we take the *last* 5 as the payload.
                if len(toks) < 5:
                    continue
                payload = toks[-5:]
                rec_type = payload[0]
                if rec_type == 0:
                    starts.append(
                        FrameStart(
                            frame_id=payload[1],
                            start_us=payload[2],
                            ifi_us=payload[3],
                            period_us=payload[4],
                        )
                    )
                elif rec_type == 1:
                    ends.append(
                        FrameEnd(
                            frame_id=payload[1],
                            end_us=payload[2],
                            dur_us=payload[3],
                            flags=payload[4],
                        )
                    )
            elif "PERF_STAGE" in line:
                toks = [parse_int(t) for t in NUM_RE.findall(line)]
                if len(toks) < 4:
                    continue
                payload = toks[-4:]
                stages.append(Stage(stage_id=payload[0], start_us=payload[1], dur_us=payload[2], aux=payload[3]))
            elif "PERF_DROP" in line:
                toks = [parse_int(t) for t in NUM_RE.findall(line)]
                if toks:
                    drops_raw.append((line.strip(), toks))

    # Correlate ends by frame_id
    ends_by_id: Dict[int, FrameEnd] = {e.frame_id: e for e in ends}

    ifi_us_list: List[int] = [s.ifi_us for s in starts if s.ifi_us > 0]
    period_us = None
    for s in starts:
        if s.period_us > 0:
            period_us = s.period_us
            break

    print("=== PERF_FRAME (inter-frame interval / jitter) ===")
    print(summarize_us(ifi_us_list, "IFI"))
    if period_us:
        # jitter as deviation from requested period
        jitter = [int(ifi - period_us) for ifi in ifi_us_list]
        print(summarize_us(jitter, f"Jitter (IFI - {period_us}us)"))

        # rough drop detection from IFI gaps
        gap_thresh = int(1.5 * period_us)
        gaps = [ifi for ifi in ifi_us_list if ifi > gap_thresh]
        print(f"Detected IFI gaps > {gap_thresh}us: {len(gaps)}")

        achieved_fps = 1_000_000.0 / mean([float(x) for x in ifi_us_list]) if ifi_us_list else math.nan
        print(f"Achieved FPS (from mean IFI): {achieved_fps:.2f}")

    # Transfer duration
    dur_us_list = [e.dur_us for e in ends]
    print("\n=== PERF_FRAME (transfer duration) ===")
    print(summarize_us(dur_us_list, "Transfer duration"))

    # Flags
    FLAG_DEFER_SEEN = 1 << 0
    FLAG_DEFER_DROPPED = 1 << 1
    defer_seen = sum(1 for e in ends if (e.flags & FLAG_DEFER_SEEN) != 0)
    defer_dropped = sum(1 for e in ends if (e.flags & FLAG_DEFER_DROPPED) != 0)
    print(f"Frames with defer seen: {defer_seen}/{len(ends)}")
    print(f"Frames with defer drop: {defer_dropped}/{len(ends)}")

    # Stages
    print("\n=== PERF_STAGE (durations by stage_id) ===")
    stage_names = {
        0: "pattern_read (SD)",
        1: "pattern_decode",
        2: "fill_frame_from_decoded",
        3: "stream_decode",
        4: "fill_all_on",
    }
    by_stage: Dict[int, List[int]] = {}
    for st in stages:
        by_stage.setdefault(st.stage_id, []).append(st.dur_us)

    for stage_id, durs in sorted(by_stage.items()):
        name = stage_names.get(stage_id, f"stage_{stage_id}")
        print(summarize_us(durs, f"{stage_id} ({name})"))

    # Drops
    print("\n=== PERF_DROP (raw) ===")
    if not drops_raw:
        print("(no PERF_DROP records found)")
    else:
        # Just print a compact summary
        post_fail = 0
        defer_drop = 0
        for _, toks in drops_raw:
            # Heuristic: the last 3 ints often include [type, ...]
            t = toks[-3:] if len(toks) >= 3 else toks
            if t and t[0] == 0:
                post_fail += 1
            if t and t[0] == 1:
                defer_drop += 1
        print(f"PERF_DROP records: {len(drops_raw)}")
        print(f"  POST_X failures: {post_fail}")
        print(f"  Defer overflows (drops): {defer_drop}")
        # If you need exact interpretation, inspect the lines:
        # for line, _ in drops_raw[:10]:
        #     print(line)


if __name__ == "__main__":
    main()
