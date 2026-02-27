#pragma once

#include <stddef.h>
#include <stdint.h>

#include "records.hpp"

#include "constants.hpp" // AC_ENABLE_PERF_PROBE default

// Performance/profiling probe
//
// Design goals:
//   * Board-agnostic core: no pin numbers or digitalWriteFast() usage here.
//     All GPIO is delegated to BSP::{perfRefreshTickToggle,
//     perfFrameTransferSet, perfFetchSet} implemented in
//     examples/<board>/bsp.cpp.
//   * Low-overhead: streaming stats with optional p95/p99 estimation.
//   * QS output emitted only at session end (e.g., pattern finished), not
//     per-frame.
//
// Integration points (called from fsp.cpp/QM actions):
//   - begin_session(...) at pattern start
//   - on_refresh_isr_post(...) in refresh ISR wrapper
//   - on_frame_start/on_frame_end around frame transfer
//   - on_panelset_start/on_panelset_end around panel-set transfers
//   - stage_begin/stage_end around SD read / decode / fill, etc.
//   - qs_report_session(...) when pattern completes

namespace Perf
{

// Backward-compatible compact payload (used by GET_PERF_STATS_CMD in earlier
// iterations). Intentionally kept small.
//
// flags:
//   bit0: any drops
//   bit1: any defers
struct PerfStatsPayload
{
  uint16_t refresh_rate_hz;
  uint16_t flags;
  uint16_t ifi_mean_us;
  uint16_t ifi_std_us;
  int16_t jitter_min_us;
  int16_t jitter_max_us;
  uint16_t frame_dur_mean_us;
  uint16_t frame_dur_max_us;
  uint16_t fetch_dur_mean_us;
  uint16_t fetch_dur_max_us;
  uint16_t drop_count;
  uint16_t defer_count;
  uint16_t ifi_n;
  uint16_t reserved;
} __attribute__ ((packed));

#if AC_ENABLE_PERF_PROBE

// Stages used for breakdown of CPU-side work during pattern playback.
// These correspond to major activities in the Pattern/Frame pipeline.
enum Stage : uint8_t
{
  STAGE_SD_READ = 0U,
  STAGE_PATTERN_DECODE,
  STAGE_FILL_FRAME_BUFFER,
  STAGE_FILL_ALL_ON,
  STAGE_STREAM_DECODE,
  STAGE_COUNT
};

enum class SessionMode : uint8_t
{
  None = 0U,
  Pattern = 1U,
  Stream = 2U,
  Other = 3U,
};

struct Snapshot
{
  SessionMode mode;
  uint16_t target_hz;
  uint32_t period_us;

  uint64_t window_us;

  uint32_t refresh_ticks;
  uint32_t refresh_post_fail;
  uint32_t refresh_defers;
  uint32_t refresh_defer_drops;

  uint32_t frames_started;
  uint32_t frames_completed;

  uint32_t late_frames;
  uint32_t max_late_us;

  // IFI distribution
  uint32_t ifi_n;
  uint32_t ifi_mean_us;
  uint32_t ifi_std_us;
  uint32_t ifi_p95_us;
  uint32_t ifi_p99_us;
  uint32_t ifi_min_us;
  uint32_t ifi_max_us;

  // Transfer distribution
  uint32_t xfer_n;
  uint32_t xfer_mean_us;
  uint32_t xfer_p99_us;
  uint32_t xfer_max_us;

  // SPI-ish portion of transfer (sum of panel-set durations per frame)
  uint32_t spi_frame_mean_us;
  uint32_t spi_frame_p99_us;
  uint32_t spi_frame_max_us;

  // Non-SPI overhead inside transfer
  uint32_t ovh_frame_mean_us;
  uint32_t ovh_frame_max_us;

  // Panel-set transfer distribution
  uint32_t panelset_n;
  uint32_t panelset_mean_us;
  uint32_t panelset_p99_us;
  uint32_t panelset_max_us;

  // Stages (SD/Decode/Fill)
  uint32_t stage_n[STAGE_COUNT];
  uint32_t stage_mean_us[STAGE_COUNT];
  uint32_t stage_p99_us[STAGE_COUNT];
  uint32_t stage_max_us[STAGE_COUNT];

  // SD spike counters (how often SD read exceeds thresholds)
  uint32_t sd_over_500us;
  uint32_t sd_over_1000us;
  uint32_t sd_over_2000us;
  uint32_t sd_over_5000us;

  // Estimates
  uint16_t safe_fps_p99_pipe;
  uint16_t safe_fps_max_pipe;
  uint32_t crit_p99_us;
  uint32_t crit_max_us;
  uint32_t guard_p99_us;
  uint32_t guard_max_us;
  bool limiter_is_transfer;
};

// Reset rolling stats/counters but keep mode metadata.
void reset_window ();

// Start a new measurement session (resets window + sets metadata).
void begin_session (SessionMode mode, uint16_t target_hz, uint32_t runtime_ms);

// Optional explicit end; currently just marks end time.
void end_session ();

// ISR-level hook: called when refresh ISR attempts to POST the refresh
// timeout. If post_ok==false, the refresh tick was dropped at the source.
void on_refresh_isr_post (bool post_ok);

// Called when a refresh timeout arrives while transferring and the firmware
// attempts to defer it. If deferred_ok==false, the defer queue overflowed and
// the refresh tick was dropped.
void on_refresh_defer (bool deferred_ok);

// Compatibility alias: older fsp.cpp called this from the Display defer path.
static inline void
on_defer_attempt (bool deferred_ok)
{
  on_refresh_defer (deferred_ok);
}

// Called when a frame transfer begins/ends.
void on_frame_start (uint16_t refresh_rate_hz);
void on_frame_end ();

// Called around each panel-set transfer.
void on_panelset_start ();
void on_panelset_end ();

// Generic nested "fetch" scope pin (can be used for any CPU prep work).
void fetch_begin ();
void fetch_end ();

// Stage-tagged timing helpers (also drive the fetch scope/pin).
void stage_begin (Stage s);
void stage_end (Stage s);

// Print a multi-line, self-explanatory QS performance report.
// Intended to be called at end-of-pattern (e.g., in
// Pattern_endRuntimeDuration) or other session end.
void qs_report_session (uint8_t qs_prio, const char *reason);

// Optional helpers (used by string/binary PERF commands).
PerfStatsPayload snapshot_payload_v1 ();

// Compatibility alias for older code.
static inline PerfStatsPayload
snapshot_payload ()
{
  return snapshot_payload_v1 ();
}

void format_summary (char *out, size_t out_len);

// Obtain a snapshot suitable for embedding in a second Ethernet response
// later.
Snapshot snapshot ();

#else // AC_ENABLE_PERF_PROBE

// No-op stubs when performance probe is disabled.
enum Stage : uint8_t
{
  STAGE_SD_READ = 0U,
  STAGE_PATTERN_DECODE,
  STAGE_FILL_FRAME_BUFFER,
  STAGE_FILL_ALL_ON,
  STAGE_STREAM_DECODE,
  STAGE_COUNT
};

enum class SessionMode : uint8_t
{
  None = 0U,
  Pattern = 1U,
  Stream = 2U,
  Other = 3U,
};

struct Snapshot
{
  SessionMode mode;
  uint16_t target_hz;
  uint32_t period_us;
  uint64_t window_us;
  uint32_t refresh_ticks;
  uint32_t refresh_post_fail;
  uint32_t refresh_defers;
  uint32_t refresh_defer_drops;
  uint32_t frames_started;
  uint32_t frames_completed;
  uint32_t late_frames;
  uint32_t max_late_us;
  uint32_t ifi_n;
  uint32_t ifi_mean_us;
  uint32_t ifi_std_us;
  uint32_t ifi_p95_us;
  uint32_t ifi_p99_us;
  uint32_t ifi_min_us;
  uint32_t ifi_max_us;
  uint32_t xfer_n;
  uint32_t xfer_mean_us;
  uint32_t xfer_p99_us;
  uint32_t xfer_max_us;
  uint32_t spi_frame_mean_us;
  uint32_t spi_frame_p99_us;
  uint32_t spi_frame_max_us;
  uint32_t ovh_frame_mean_us;
  uint32_t ovh_frame_max_us;
  uint32_t panelset_n;
  uint32_t panelset_mean_us;
  uint32_t panelset_p99_us;
  uint32_t panelset_max_us;
  uint32_t stage_n[STAGE_COUNT];
  uint32_t stage_mean_us[STAGE_COUNT];
  uint32_t stage_p99_us[STAGE_COUNT];
  uint32_t stage_max_us[STAGE_COUNT];
  // SD spike counters
  uint32_t sd_over_500us;
  uint32_t sd_over_1000us;
  uint32_t sd_over_2000us;
  uint32_t sd_over_5000us;
  uint16_t safe_fps_p99_pipe;
  uint16_t safe_fps_max_pipe;
  uint32_t crit_p99_us;
  uint32_t crit_max_us;
  uint32_t guard_p99_us;
  uint32_t guard_max_us;
  bool limiter_is_transfer;
};

static inline void
reset_window ()
{
}
static inline void
begin_session (SessionMode, uint16_t, uint32_t)
{
}
static inline void
end_session ()
{
}
static inline void
on_refresh_isr_post (bool)
{
}
static inline void
on_refresh_defer (bool)
{
}
static inline void
on_defer_attempt (bool)
{
}
static inline void
on_frame_start (uint16_t)
{
}
static inline void
on_frame_end ()
{
}
static inline void
on_panelset_start ()
{
}
static inline void
on_panelset_end ()
{
}
static inline void
fetch_begin ()
{
}
static inline void
fetch_end ()
{
}
static inline void
stage_begin (Stage)
{
}
static inline void
stage_end (Stage)
{
}
static inline void
qs_report_session (uint8_t, const char *)
{
}
static inline PerfStatsPayload
snapshot_payload_v1 ()
{
  return PerfStatsPayload{};
}
static inline PerfStatsPayload
snapshot_payload ()
{
  return PerfStatsPayload{};
}
static inline void
format_summary (char *, size_t)
{
}
static inline Snapshot
snapshot ()
{
  return Snapshot{};
}

#endif // AC_ENABLE_PERF_PROBE

} // namespace Perf
