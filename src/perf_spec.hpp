#pragma once

// perf_spec.hpp
//
// Project-local performance probe specification.
//
// This file is intentionally **ArenaController-specific**: it defines which
// metrics and stages exist, what their short labels are, and which ones should
// enable streaming quantile estimation (p95/p99).
//
// Keeping these lists in one place makes it easy to:
//   * add/remove stages without editing multiple switch statements
//   * enable/disable quantiles per metric/stage without paying extra RAM
//   * reuse the perf_core + perf_port layers across future QP projects

// -----------------------------
// Rolling metrics
//
// Each entry: X(var_name, quantiles_enabled)
//
// The implementation will declare:
//   * optional QuantilePair q_<var_name>  (only if quantiles_enabled==1)
//   * Metric <var_name>_us               (QuantilePair-backed if enabled)
//
// NOTE: Keep var_name stable unless you also update internal references in
// perf.cpp.
#define AC_PERF_METRICS(X)                                                    \
  X (ifi, 1)                                                                  \
  X (xfer, 1)                                                                 \
  X (spi_frame, 1)                                                            \
  X (ovh_frame, 0)                                                            \
  X (panelset, 1)                                                             \
  X (fetch_scope, 1)

// -----------------------------
// CPU stages
//
// Each entry: X(enum_name, short_label, always_print, quantiles_enabled)
//
// - enum_name must match the Stage enum entries used throughout the project.
// - short_label is used in PERF_CPU reporting.
// - always_print controls whether a stage is printed even when unused.
// - quantiles_enabled controls whether a streaming p99 is computed for that
//   stage. (Only enable where tails matter.)
#define AC_PERF_STAGES(X)                                                     \
  X (STAGE_SD_READ, "SD", 1, 1)                                               \
  X (STAGE_PATTERN_DECODE, "DEC", 1, 0)                                       \
  X (STAGE_FILL_FRAME_BUFFER, "FILL", 1, 0)                                   \
  X (STAGE_FILL_ALL_ON, "ALLON", 0, 0)                                        \
  X (STAGE_STREAM_DECODE, "SDEC", 0, 0)

// -----------------------------
// Host-driven "updates" (distinct from the continuous refresh transfer)
//
// Each entry: X(enum_id, short_label)
//
// Notes:
//   * "SPF" corresponds to SET_FRAME_POSITION_CMD / UPDATE_PATTERN_FRAME.
//   * "STREAM" corresponds to streaming frame packets.
#define AC_PERF_UPDATES(X)                                                    \
  X (UPD_SHOW_PATTERN_FRAME, "SPF")                                          \
  X (UPD_STREAM_FRAME, "STREAM")
