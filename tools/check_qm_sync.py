#!/usr/bin/env python3
"""Sanity-check that ArenaController.qm contains the model changes relied on by
current generated sources.

This is intentionally narrow: it checks the specific high-value edits that would
be lost if the QM model diverged from the generated code.
"""

from __future__ import annotations

import sys
import xml.etree.ElementTree as ET
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
QM_PATH = REPO_ROOT / "src" / "ArenaController.qm"


def _find_class(root: ET.Element, name: str) -> ET.Element:
    for cls in root.iter("class"):
        if cls.attrib.get("name") == name:
            return cls
    raise RuntimeError(f"class {name!r} not found")


def _find_state(root_state: ET.Element, name: str) -> ET.Element | None:
    if root_state.attrib.get("name") == name:
        return root_state
    for child in root_state.findall("state"):
        found = _find_state(child, name)
        if found is not None:
            return found
    return None


def _require(condition: bool, msg: str, errors: list[str]) -> None:
    if not condition:
        errors.append(msg)


def main() -> int:
    root = ET.parse(QM_PATH).getroot()
    errors: list[str] = []

    eci = _find_class(root, "EthernetCommandInterface")
    eci_attrs = {a.attrib.get("name") for a in eci.findall("attribute")}
    _require("hot_poll_until_ms_" in eci_attrs,
             "EthernetCommandInterface is missing hot_poll_until_ms_ in ArenaController.qm",
             errors)
    _require("binary_command_byte_count_claim_" not in eci_attrs,
             "EthernetCommandInterface still models binary_command_byte_count_claim_ even though QNEthernet delivers complete Ethernet commands",
             errors)

    eci_sc = eci.find("statechart")
    _require(eci_sc is not None,
             "EthernetCommandInterface statechart missing in ArenaController.qm",
             errors)
    if eci_sc is not None:
        processing_stream = None
        waiting_for_new = None
        choosing = None
        for top in eci_sc.findall("state"):
            if waiting_for_new is None:
                waiting_for_new = _find_state(top, "WaitingForNewCommand")
            if processing_stream is None:
                processing_stream = _find_state(top, "ProcessingStreamCommand")
            if choosing is None:
                choosing = _find_state(top, "ChoosingCommandProcessor")
        _require(choosing is None,
                 "EthernetCommandInterface still models ChoosingCommandProcessor in ArenaController.qm",
                 errors)
        _require(waiting_for_new is not None,
                 "EthernetCommandInterface::WaitingForNewCommand missing in ArenaController.qm",
                 errors)
        if waiting_for_new is not None:
            waiting_trigs = {tran.attrib.get("trig") for tran in waiting_for_new.findall("tran")}
            _require("PROCESS_BINARY_COMMAND" in waiting_trigs,
                     "WaitingForNewCommand does not route PROCESS_BINARY_COMMAND directly in ArenaController.qm",
                     errors)
            _require("PROCESS_STREAM_COMMAND" in waiting_trigs,
                     "WaitingForNewCommand does not route PROCESS_STREAM_COMMAND directly in ArenaController.qm",
                     errors)
        _require(processing_stream is not None,
                 "EthernetCommandInterface::ProcessingStreamCommand missing in ArenaController.qm",
                 errors)
        if processing_stream is not None:
            entry_code = processing_stream.findtext("entry") or ""
            _require("EthernetCommandInterface_processStreamCommand" in entry_code,
                     "ProcessingStreamCommand entry action does not process the complete QNEthernet stream command",
                     errors)
            _require(processing_stream.find("exit") is None,
                     "ProcessingStreamCommand should no longer model stream-specific timer exit handling",
                     errors)
            child_states = {s.attrib.get("name") for s in processing_stream.findall("state")}
            _require("WaitingForCommand" not in child_states and "MidStreamCommand" not in child_states,
                     "ProcessingStreamCommand still contains Mongoose-era substates in ArenaController.qm",
                     errors)

    pattern = _find_class(root, "Pattern")
    pattern_attrs = {a.attrib.get("name") for a in pattern.findall("attribute")}
    _require("spf_pending_frame_index_" in pattern_attrs,
             "Pattern is missing spf_pending_frame_index_ in ArenaController.qm",
             errors)
    _require("spf_update_pending_" in pattern_attrs,
             "Pattern is missing spf_update_pending_ in ArenaController.qm",
             errors)

    pattern_ctor = pattern.find("operation[@name='Pattern']/code")
    ctor_code = pattern_ctor.text if pattern_ctor is not None and pattern_ctor.text else ""
    _require("spf_pending_frame_index_ = 0U;" in ctor_code,
             "Pattern constructor in ArenaController.qm does not initialize spf_pending_frame_index_",
             errors)
    _require("spf_update_pending_ = 0U;" in ctor_code,
             "Pattern constructor in ArenaController.qm does not initialize spf_update_pending_",
             errors)

    sc = pattern.find("statechart")
    _require(sc is not None, "Pattern statechart missing in ArenaController.qm", errors)
    if sc is not None:
        showing = None
        spf_displaying = None
        for top in sc.findall("state"):
            showing = _find_state(top, "ShowingPatternFrame")
            if showing is not None:
                spf_displaying = _find_state(showing, "SPF_DisplayingFrame")
                break

        _require(showing is not None,
                 "Pattern::ShowingPatternFrame state missing in ArenaController.qm",
                 errors)
        if showing is not None:
            exit_code = showing.findtext("exit") or ""
            _require("Pattern_clearPendingPatternFrameUpdate" in exit_code,
                     "ShowingPatternFrame exit action does not clear pending SPF update state",
                     errors)
            _require("Pattern_deleteFrameReference" in exit_code,
                     "ShowingPatternFrame exit action does not delete frame reference",
                     errors)

            update_tran = None
            for tran in showing.findall("tran"):
                if tran.attrib.get("trig") == "UPDATE_PATTERN_FRAME":
                    update_tran = tran
                    break
            _require(update_tran is not None,
                     "ShowingPatternFrame is missing UPDATE_PATTERN_FRAME handling in ArenaController.qm",
                     errors)
            if update_tran is not None:
                update_action = update_tran.findtext("action") or ""
                _require("Pattern_storePendingPatternFrameUpdate" in update_action,
                         "ShowingPatternFrame UPDATE_PATTERN_FRAME action does not store the pending SPF update",
                         errors)

        _require(spf_displaying is not None,
                 "Pattern::SPF_DisplayingFrame state missing in ArenaController.qm",
                 errors)
        if spf_displaying is not None:
            ft = None
            for tran in spf_displaying.findall("tran"):
                if tran.attrib.get("trig") == "FRAME_TRANSFERRED":
                    ft = tran
                    break
            _require(ft is not None,
                     "SPF_DisplayingFrame is missing FRAME_TRANSFERRED handling in ArenaController.qm",
                     errors)
            if ft is not None:
                choices = ft.findall("choice")
                _require(len(choices) >= 2,
                         "SPF_DisplayingFrame FRAME_TRANSFERRED is not modeled as a guarded choice",
                         errors)
                if len(choices) >= 2:
                    first_guard = choices[0].findtext("guard") or ""
                    first_action = choices[0].findtext("action") or ""
                    second_guard = choices[1].findtext("guard") or ""
                    _require("Pattern_hasPendingPatternFrameUpdate" in first_guard,
                             "SPF_DisplayingFrame first choice does not test pending SPF updates",
                             errors)
                    _require("Pattern_applyPendingPatternFrameUpdate" in first_action,
                             "SPF_DisplayingFrame first choice does not apply the pending SPF update",
                             errors)
                    _require(second_guard.strip() == "",
                             "SPF_DisplayingFrame second choice should be the else branch",
                             errors)

    if errors:
        for msg in errors:
            print(f"ERROR: {msg}")
        return 1

    print("ArenaController.qm contains the expected SPF pending-update and QNEthernet-only Ethernet model changes.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
