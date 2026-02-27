#include "perf.hpp"

#if AC_ENABLE_PERF_PROBE

#include <stdint.h>

#include "perf/perf_core.hpp"
#include "perf/perf_port.hpp"

namespace Perf
{

// Saturating helpers for compact binary responses
static inline uint16_t
sat_u16 (uint32_t v)
{
  return (v > 0xFFFFu) ? 0xFFFFu : static_cast<uint16_t> (v);
}

static inline int16_t
sat_i16 (int32_t v)
{
  if (v > 32767)
    {
      return 32767;
    }
  if (v < -32768)
    {
      return -32768;
    }
  return static_cast<int16_t> (v);
}

// -----------------------------
// Global profiler state

static SessionMode s_mode = SessionMode::None;
static uint16_t s_target_hz = 0U;
static uint32_t s_runtime_ms = 0U;
static uint32_t s_period_us = 0U;

static uint64_t first_frame_start_us = 0ULL;
static uint64_t last_frame_end_us = 0ULL;

// Extend micros() to 64-bit so long sessions (e.g., 109 minutes) do not
// break window_ms/window_us calculations when micros() wraps (~71 minutes).
static core::Micros64Extender micros64;

static uint32_t last_frame_start_us = 0U;
static uint32_t current_frame_start_us = 0U;

static uint32_t current_frame_panelset_sum_us = 0U;
static uint32_t panelset_start_us = 0U;

// Counters
static volatile uint32_t refresh_tick_count = 0U;
static volatile uint32_t refresh_post_fail_count = 0U;
static volatile uint32_t refresh_defer_count = 0U;
static volatile uint32_t refresh_defer_drop_count = 0U;

static uint32_t frames_started = 0U;
static uint32_t frames_completed = 0U;

static uint32_t late_frame_count = 0U;
static uint32_t late_max_us = 0U;

// SD spike counters (counts of SD read durations above thresholds)
static uint32_t sd_over_500us_count = 0U;
static uint32_t sd_over_1000us_count = 0U;
static uint32_t sd_over_2000us_count = 0U;
static uint32_t sd_over_5000us_count = 0U;

// -----------------------------
// Host-driven update tracking

struct UpdateTracker
{
  uint32_t received = 0U;
  uint32_t processed = 0U;
  uint32_t committed = 0U;
  uint32_t applied = 0U;
  uint32_t coalesced = 0U;

  core::QuantilePair q_ifi;
  core::QuantilePair q_latency;
  core::Metric ifi_us{ &q_ifi };
  core::Metric latency_us{ &q_latency };

  static constexpr uint8_t kProcFifoSize = 8U;
  uint64_t proc_ts_us[kProcFifoSize]{};
  uint8_t proc_head = 0U;
  uint8_t proc_len = 0U;

  // The next frame-buffer swap(s) are expected to correspond to this update
  // kind. This is a count to handle multiple in-flight updates.
  uint8_t commit_expected = 0U;

  // A committed update waiting to become visible on a subsequent refresh.
  bool pending_valid = false;
  bool pending_transfer_started = false;
  uint32_t pending_frame_started_at_commit = 0U;
  uint64_t pending_processed_ts_us64 = 0ULL;

  uint64_t last_applied_us64 = 0ULL;

  void
  reset ()
  {
    received = 0U;
    processed = 0U;
    committed = 0U;
    applied = 0U;
    coalesced = 0U;
    ifi_us.reset ();
    latency_us.reset ();
    proc_head = 0U;
    proc_len = 0U;
    for (uint8_t i = 0U; i < kProcFifoSize; ++i)
      {
        proc_ts_us[i] = 0ULL;
      }
    commit_expected = 0U;
    pending_valid = false;
    pending_transfer_started = false;
    pending_frame_started_at_commit = 0U;
    pending_processed_ts_us64 = 0ULL;
    last_applied_us64 = 0ULL;
  }

  void
  proc_push (uint64_t ts_us64)
  {
    if (proc_len == kProcFifoSize)
      {
        // Tracking FIFO overflow. Drop the oldest timestamp and count it as
        // coalesced (the update won't have a matching latency measurement).
        proc_head = static_cast<uint8_t> ((proc_head + 1U) % kProcFifoSize);
        --proc_len;
        ++coalesced;
      }
    uint8_t const tail
        = static_cast<uint8_t> ((proc_head + proc_len) % kProcFifoSize);
    proc_ts_us[tail] = ts_us64;
    ++proc_len;
  }

  bool
  proc_pop (uint64_t &out_ts_us64)
  {
    if (proc_len == 0U)
      {
        return false;
      }
    out_ts_us64 = proc_ts_us[proc_head];
    proc_head = static_cast<uint8_t> ((proc_head + 1U) % kProcFifoSize);
    --proc_len;
    return true;
  }
};

static UpdateTracker updates[UPD_COUNT];

// -----------------------------
// Metrics

// Use the project spec list (perf_spec.hpp) to declare metrics and their
// optional quantiles in one place.

// Small macro helpers for "boolean" parameters in the spec list.
#define PERF_SPEC_IF_1(_code) _code
#define PERF_SPEC_IF_0(_code)

#define PERF_SPEC_QPTR_1(_name) (&q_##_name)
#define PERF_SPEC_QPTR_0(_name) (nullptr)

// Declare QuantilePair storage only for metrics that enable quantiles.
#define PERF_SPEC_DECL_METRIC_Q(_name, _quant)                                \
  PERF_SPEC_IF_##_quant (static core::QuantilePair q_##_name;)
AC_PERF_METRICS (PERF_SPEC_DECL_METRIC_Q)
#undef PERF_SPEC_DECL_METRIC_Q

// Declare Metric objects for all configured metrics.
#define PERF_SPEC_DECL_METRIC(_name, _quant)                                  \
  static core::Metric _name##_us (PERF_SPEC_QPTR_##_quant (_name));
AC_PERF_METRICS (PERF_SPEC_DECL_METRIC)
#undef PERF_SPEC_DECL_METRIC

static uint16_t fetch_depth = 0U;
static uint32_t fetch_start_us = 0U;

// Declare QuantilePair storage only for stages that enable quantiles.
#define PERF_SPEC_DECL_STAGE_Q(_id, _label, _always, _quant)                  \
  PERF_SPEC_IF_##_quant (static core::QuantilePair q_##_id;)
AC_PERF_STAGES (PERF_SPEC_DECL_STAGE_Q)
#undef PERF_SPEC_DECL_STAGE_Q

// Declare StageMetric array in the exact order of the Stage enum.
static core::StageMetric stages[STAGE_COUNT] = {
#define PERF_SPEC_STAGE_INIT(_id, _label, _always, _quant)                    \
  core::StageMetric (PERF_SPEC_QPTR_##_quant (_id)),
  AC_PERF_STAGES (PERF_SPEC_STAGE_INIT)
#undef PERF_SPEC_STAGE_INIT
};

// Clean up internal macro helpers.
#undef PERF_SPEC_QPTR_1
#undef PERF_SPEC_QPTR_0
#undef PERF_SPEC_IF_1
#undef PERF_SPEC_IF_0

static inline uint32_t
hz_to_period_us (uint16_t hz)
{
  return (hz != 0U) ? (1000000UL / static_cast<uint32_t> (hz)) : 0U;
}

static inline uint16_t
safe_div_u16 (uint32_t numer, uint32_t denom)
{
  if (denom == 0U)
    {
      return 0U;
    }
  uint32_t const q = numer / denom;
  return (q > 0xFFFFu) ? 0xFFFFu : static_cast<uint16_t> (q);
}

void
reset_window ()
{
  // timestamps
  first_frame_start_us = 0ULL;
  last_frame_end_us = 0ULL;
  last_frame_start_us = 0U;
  current_frame_start_us = 0U;

  micros64.reset ();

  current_frame_panelset_sum_us = 0U;
  panelset_start_us = 0U;

  // counters
  refresh_tick_count = 0U;
  refresh_post_fail_count = 0U;
  refresh_defer_count = 0U;
  refresh_defer_drop_count = 0U;

  frames_started = 0U;
  frames_completed = 0U;

  late_frame_count = 0U;
  late_max_us = 0U;

  sd_over_500us_count = 0U;
  sd_over_1000us_count = 0U;
  sd_over_2000us_count = 0U;
  sd_over_5000us_count = 0U;

  // metrics
  // Keep metric resets in sync with perf_spec.hpp.
#define PERF_SPEC_RESET_METRIC(_name, _quant) _name##_us.reset ();
  AC_PERF_METRICS (PERF_SPEC_RESET_METRIC)
#undef PERF_SPEC_RESET_METRIC

  fetch_depth = 0U;
  fetch_start_us = 0U;

  for (uint8_t i = 0U; i < STAGE_COUNT; ++i)
    {
      stages[i].reset ();
    }

  for (uint8_t i = 0U; i < UPD_COUNT; ++i)
    {
      updates[i].reset ();
    }
}

void
begin_session (SessionMode mode, uint16_t target_hz, uint32_t runtime_ms)
{
  s_mode = mode;
  s_target_hz = target_hz;
  s_runtime_ms = runtime_ms;
  s_period_us = hz_to_period_us (target_hz);
  reset_window ();
}


void
end_session ()
{
  // Nothing special yet. Window is derived from first/last frame.
}

void
on_refresh_isr_post (bool post_ok)
{
  ++refresh_tick_count;
  // scope pin: tick edge marker
  port::refresh_tick_toggle ();
  if (!post_ok)
    {
      ++refresh_post_fail_count;
    }
}

void
on_refresh_defer (bool deferred_ok)
{
  ++refresh_defer_count;
  if (!deferred_ok)
    {
      ++refresh_defer_drop_count;
    }
}

void
on_frame_start (uint16_t refresh_rate_hz)
{
  ++frames_started;

  uint32_t const start_us = port::now_us32 ();
  uint64_t const start_us64 = micros64.extend (start_us);
  current_frame_start_us = start_us;
  current_frame_panelset_sum_us = 0U;
  panelset_start_us = 0U;

  // Update expected period dynamically if refresh rate changes mid-session.
  if (refresh_rate_hz != 0U)
    {
      s_period_us = hz_to_period_us (refresh_rate_hz);
      s_target_hz = refresh_rate_hz;
    }

  port::frame_transfer_set (true);

  if (first_frame_start_us == 0ULL)
    {
      first_frame_start_us = start_us64;
    }

  if (last_frame_start_us != 0U)
    {
      uint32_t const ifi = start_us - last_frame_start_us;
      ifi_us.push_u32 (ifi);
    }
  last_frame_start_us = start_us;

  // If an update was committed while a transfer was already in progress, it
  // should be attributed to the *next* transfer that starts after the commit.
  for (uint8_t i = 0U; i < UPD_COUNT; ++i)
    {
      UpdateTracker &u = updates[i];
      if (u.pending_valid && !u.pending_transfer_started
          && frames_started > u.pending_frame_started_at_commit)
        {
          u.pending_transfer_started = true;
        }
    }
}

void
on_frame_end ()
{
  ++frames_completed;

  uint32_t const end_us = port::now_us32 ();
  uint64_t const end_us64 = micros64.extend (end_us);
  last_frame_end_us = end_us64;

  uint32_t const dur_us = end_us - current_frame_start_us;
  xfer_us.push_u32 (dur_us);

  spi_frame_us.push_u32 (current_frame_panelset_sum_us);

  uint32_t const overhead_us = (dur_us >= current_frame_panelset_sum_us)
                                   ? (dur_us - current_frame_panelset_sum_us)
                                   : 0U;
  ovh_frame_us.push_u32 (overhead_us);

  if (s_period_us != 0U && dur_us > s_period_us)
    {
      ++late_frame_count;
      uint32_t const late_us = dur_us - s_period_us;
      if (late_us > late_max_us)
        {
          late_max_us = late_us;
        }
    }

  // Update apply point: first completed transfer after a committed buffer swap.
  for (uint8_t i = 0U; i < UPD_COUNT; ++i)
    {
      UpdateTracker &u = updates[i];
      if (u.pending_valid && u.pending_transfer_started)
        {
          ++u.applied;

          uint64_t latency_us64 = 0ULL;
          if (end_us64 >= u.pending_processed_ts_us64)
            {
              latency_us64 = end_us64 - u.pending_processed_ts_us64;
            }
          uint32_t const latency_u32
              = (latency_us64 > 0xFFFFFFFFULL)
                    ? 0xFFFFFFFFu
                    : static_cast<uint32_t> (latency_us64);
          u.latency_us.push_u32 (latency_u32);

          if (u.last_applied_us64 != 0ULL)
            {
              uint64_t const ifi_us64 = end_us64 - u.last_applied_us64;
              uint32_t const ifi_u32
                  = (ifi_us64 > 0xFFFFFFFFULL)
                        ? 0xFFFFFFFFu
                        : static_cast<uint32_t> (ifi_us64);
              u.ifi_us.push_u32 (ifi_u32);
            }
          u.last_applied_us64 = end_us64;

          u.pending_valid = false;
          u.pending_transfer_started = false;
          u.pending_frame_started_at_commit = 0U;
          u.pending_processed_ts_us64 = 0ULL;
        }
    }

  port::frame_transfer_set (false);
}

void
on_panelset_start ()
{
  panelset_start_us = port::now_us32 ();
}

void
on_panelset_end ()
{
  if (panelset_start_us == 0U)
    {
      return;
    }
  uint32_t const end_us = port::now_us32 ();
  uint32_t const dur_us = end_us - panelset_start_us;
  panelset_us.push_u32 (dur_us);
  current_frame_panelset_sum_us += dur_us;
  panelset_start_us = 0U;
}

void
fetch_begin ()
{
  if (fetch_depth == 0U)
    {
      fetch_start_us = port::now_us32 ();
      port::fetch_set (true);
    }
  ++fetch_depth;
}

void
fetch_end ()
{
  if (fetch_depth == 0U)
    {
      return;
    }
  --fetch_depth;
  if (fetch_depth == 0U)
    {
      port::fetch_set (false);
      uint32_t const end_us = port::now_us32 ();
      uint32_t const dur_us = end_us - fetch_start_us;
      fetch_scope_us.push_u32 (dur_us);
    }
}

void
stage_begin (Stage s)
{
  fetch_begin ();
  uint8_t const idx = static_cast<uint8_t> (s);
  if (idx >= STAGE_COUNT)
    {
      return;
    }
  if (stages[idx].depth == 0U)
    {
      stages[idx].start_us = port::now_us32 ();
    }
  ++stages[idx].depth;
}

void
stage_end (Stage s)
{
  uint8_t const idx = static_cast<uint8_t> (s);
  if (idx < STAGE_COUNT && stages[idx].depth != 0U)
    {
      --stages[idx].depth;
      if (stages[idx].depth == 0U)
        {
          uint32_t const end_us = port::now_us32 ();
          uint32_t const dur_us = end_us - stages[idx].start_us;
          stages[idx].dur_us.push_u32 (dur_us);

          if (idx == STAGE_SD_READ)
            {
              if (dur_us > 500U)
                {
                  ++sd_over_500us_count;
                }
              if (dur_us > 1000U)
                {
                  ++sd_over_1000us_count;
                }
              if (dur_us > 2000U)
                {
                  ++sd_over_2000us_count;
                }
              if (dur_us > 5000U)
                {
                  ++sd_over_5000us_count;
                }
            }
        }
    }
  fetch_end ();
}

void
update_received (UpdateKind k)
{
  uint8_t const idx = static_cast<uint8_t> (k);
  if (idx >= UPD_COUNT)
    {
      return;
    }
  ++updates[idx].received;
}

void
update_processed (UpdateKind k)
{
  uint8_t const idx = static_cast<uint8_t> (k);
  if (idx >= UPD_COUNT)
    {
      return;
    }
  ++updates[idx].processed;
  uint32_t const now_us32 = port::now_us32 ();
  uint64_t const now_us64 = micros64.extend (now_us32);
  updates[idx].proc_push (now_us64);
}

void
update_expect_commit (UpdateKind k)
{
  uint8_t const idx = static_cast<uint8_t> (k);
  if (idx >= UPD_COUNT)
    {
      return;
    }
  if (updates[idx].commit_expected != 0xFFU)
    {
      ++updates[idx].commit_expected;
    }
}

void
on_frame_reference_saved ()
{
  // A new frame reference has been installed by the Frame AO.
  // If any update kind is expecting a commit, treat this as the point where
  // the update becomes eligible to appear on a subsequent refresh.

  uint8_t idx = 0xFFU;
  for (uint8_t i = 0U; i < UPD_COUNT; ++i)
    {
      if (updates[i].commit_expected != 0U)
        {
          idx = i;
          break;
        }
    }

  if (idx == 0xFFU)
    {
      return;
    }

  UpdateTracker &u = updates[idx];
  --u.commit_expected;
  ++u.committed;

  uint64_t processed_ts = 0ULL;
  if (!u.proc_pop (processed_ts))
    {
      // Fallback: no processed timestamp captured; use commit time.
      uint32_t const now_us32 = port::now_us32 ();
      processed_ts = micros64.extend (now_us32);
    }

  if (u.pending_valid)
    {
      // A previous committed update was never observed on the wire before the
      // buffer was swapped again. Count it as coalesced.
      ++u.coalesced;
    }

  u.pending_valid = true;
  u.pending_transfer_started = false;
  u.pending_frame_started_at_commit = frames_started;
  u.pending_processed_ts_us64 = processed_ts;
}

// Compute derived window_us
static inline uint64_t
window_us ()
{
  if (first_frame_start_us == 0ULL || last_frame_end_us == 0ULL
      || last_frame_end_us < first_frame_start_us)
    {
      return 0ULL;
    }
  return last_frame_end_us - first_frame_start_us;
}

static Snapshot
compute_snapshot ()
{
  Snapshot s{};
  s.mode = s_mode;
  s.target_hz = s_target_hz;
  s.period_us = s_period_us;
  s.runtime_ms = s_runtime_ms;
  s.window_us = window_us ();

  s.refresh_ticks = refresh_tick_count;
  s.refresh_post_fail = refresh_post_fail_count;
  s.refresh_defers = refresh_defer_count;
  s.refresh_defer_drops = refresh_defer_drop_count;

  s.frames_started = frames_started;
  s.frames_completed = frames_completed;

  s.late_frames = late_frame_count;
  s.max_late_us = late_max_us;

  s.ifi_n = ifi_us.stats.n;
  s.ifi_mean_us = ifi_us.mean_u32 ();
  s.ifi_std_us = ifi_us.std_u32 ();
  s.ifi_p95_us = ifi_us.p95_u32 ();
  s.ifi_p99_us = ifi_us.p99_u32 ();
  s.ifi_min_us = ifi_us.min_u32 ();
  s.ifi_max_us = ifi_us.max_u32 ();

  s.xfer_n = xfer_us.stats.n;
  s.xfer_mean_us = xfer_us.mean_u32 ();
  s.xfer_p99_us = xfer_us.p99_u32 ();
  s.xfer_max_us = xfer_us.max_u32 ();
  s.xfer_sum_us = xfer_us.sum_us;

  s.spi_frame_mean_us = spi_frame_us.mean_u32 ();
  s.spi_frame_p99_us = spi_frame_us.p99_u32 ();
  s.spi_frame_max_us = spi_frame_us.max_u32 ();
  s.spi_frame_sum_us = spi_frame_us.sum_us;

  s.ovh_frame_mean_us = ovh_frame_us.mean_u32 ();
  s.ovh_frame_max_us = ovh_frame_us.max_u32 ();
  s.ovh_frame_sum_us = ovh_frame_us.sum_us;

  s.panelset_n = panelset_us.stats.n;
  s.panelset_mean_us = panelset_us.mean_u32 ();
  s.panelset_p99_us = panelset_us.p99_u32 ();
  s.panelset_max_us = panelset_us.max_u32 ();
  s.panelset_sum_us = panelset_us.sum_us;

  s.fetch_n = fetch_scope_us.stats.n;
  s.fetch_mean_us = fetch_scope_us.mean_u32 ();
  s.fetch_p99_us = fetch_scope_us.p99_u32 ();
  s.fetch_max_us = fetch_scope_us.max_u32 ();
  s.fetch_sum_us = fetch_scope_us.sum_us;

  s.sd_over_500us = sd_over_500us_count;
  s.sd_over_1000us = sd_over_1000us_count;
  s.sd_over_2000us = sd_over_2000us_count;
  s.sd_over_5000us = sd_over_5000us_count;

  for (uint8_t i = 0U; i < STAGE_COUNT; ++i)
    {
      s.stage_n[i] = stages[i].dur_us.stats.n;
      s.stage_mean_us[i] = stages[i].dur_us.mean_u32 ();
      // "p99-ish" value: if a true streaming p99 is enabled+ready, use it;
      // otherwise fall back to max observed. This keeps CPU-limit estimates
      // meaningful even for deterministic stages where quantiles are disabled.
      s.stage_p99_us[i] = stages[i].dur_us.p99_ready ()
                              ? stages[i].dur_us.p99_u32 ()
                              : stages[i].dur_us.max_u32 ();
      s.stage_max_us[i] = stages[i].dur_us.max_u32 ();
      s.stage_sum_us[i] = stages[i].dur_us.sum_us;
    }

  // Safe FPS estimate (pipelined model): period >= max(xfer, cpu) + guard
  // CPU estimate uses sum of per-frame stage p99/max (conservative).
  uint32_t cpu_p99 = 0U;
  uint32_t cpu_max = 0U;
  for (uint8_t i = 0U; i < STAGE_COUNT; ++i)
    {
      cpu_p99 += s.stage_p99_us[i];
      cpu_max += s.stage_max_us[i];
    }

  uint32_t const xfer_p99
      = (xfer_us.p99_ready () ? s.xfer_p99_us : s.xfer_max_us);

  uint32_t crit_p99 = xfer_p99;
  bool limiter_transfer = true;
  if (cpu_p99 > crit_p99)
    {
      crit_p99 = cpu_p99;
      limiter_transfer = false;
    }

  uint32_t crit_max = s.xfer_max_us;
  if (cpu_max > crit_max)
    {
      crit_max = cpu_max;
    }

  uint32_t guard_p99 = crit_p99 / 10U; // 10%
  if (guard_p99 < 200U)
    {
      guard_p99 = 200U;
    }
  uint32_t guard_max = crit_max / 10U;
  if (guard_max < 200U)
    {
      guard_max = 200U;
    }

  s.safe_fps_p99_pipe = safe_div_u16 (1000000UL, (crit_p99 + guard_p99));
  s.safe_fps_max_pipe = safe_div_u16 (1000000UL, (crit_max + guard_max));

  s.crit_p99_us = crit_p99;
  s.crit_max_us = crit_max;
  s.guard_p99_us = guard_p99;
  s.guard_max_us = guard_max;
  s.limiter_is_transfer = limiter_transfer;

  for (uint8_t i = 0U; i < UPD_COUNT; ++i)
    {
      s.upd[i].received = updates[i].received;
      s.upd[i].processed = updates[i].processed;
      s.upd[i].committed = updates[i].committed;
      s.upd[i].applied = updates[i].applied;
      s.upd[i].coalesced = updates[i].coalesced;

      s.upd[i].ifi_n = updates[i].ifi_us.stats.n;
      s.upd[i].ifi_mean_us = updates[i].ifi_us.mean_u32 ();
      s.upd[i].ifi_p99_us = updates[i].ifi_us.p99_u32 ();
      s.upd[i].ifi_max_us = updates[i].ifi_us.max_u32 ();

      s.upd[i].latency_n = updates[i].latency_us.stats.n;
      s.upd[i].latency_mean_us = updates[i].latency_us.mean_u32 ();
      s.upd[i].latency_p99_us = updates[i].latency_us.p99_u32 ();
      s.upd[i].latency_max_us = updates[i].latency_us.max_u32 ();
    }

  return s;
}

Snapshot
snapshot ()
{
  return compute_snapshot ();
}

PerfStatsPayload
snapshot_payload_v1 ()
{
  Snapshot const s = compute_snapshot ();
  PerfStatsPayload p{};

  uint32_t const drops_total = s.refresh_post_fail + s.refresh_defer_drops;

  // jitter is derived from IFI min/max relative to expected period
  int32_t jitter_min = 0;
  int32_t jitter_max = 0;
  if (s.ifi_n != 0U && s.period_us != 0U)
    {
      jitter_min = static_cast<int32_t> (s.ifi_min_us)
                   - static_cast<int32_t> (s.period_us);
      jitter_max = static_cast<int32_t> (s.ifi_max_us)
                   - static_cast<int32_t> (s.period_us);
    }

  p.refresh_rate_hz = sat_u16 (s.target_hz);
  p.flags = 0U;
  if (drops_total != 0U)
    {
      p.flags |= 0x0001U;
    }
  if (s.refresh_defers != 0U)
    {
      p.flags |= 0x0002U;
    }

  p.ifi_mean_us = sat_u16 (s.ifi_mean_us);
  p.ifi_std_us = sat_u16 (s.ifi_std_us);
  p.jitter_min_us = sat_i16 (jitter_min);
  p.jitter_max_us = sat_i16 (jitter_max);

  p.frame_dur_mean_us = sat_u16 (s.xfer_mean_us);
  p.frame_dur_max_us = sat_u16 (s.xfer_max_us);

  p.fetch_dur_mean_us = sat_u16 (s.fetch_mean_us);
  p.fetch_dur_max_us = sat_u16 (s.fetch_max_us);

  p.drop_count = sat_u16 (drops_total);
  p.defer_count = sat_u16 (s.refresh_defers);
  p.ifi_n = sat_u16 (s.ifi_n);
  p.reserved = 0U;

  return p;
}

} // namespace Perf

#endif // AC_ENABLE_PERF_PROBE
