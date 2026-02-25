#include "perf.hpp"

#if AC_ENABLE_PERF_PROBE

#include <Arduino.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "bsp.hpp"
#include "qpcpp.hpp"

using namespace QP;

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
// Streaming stats

struct RunningStats
{
  uint32_t n = 0U;
  double mean = 0.0;
  double m2 = 0.0;
  double min = 0.0;
  double max = 0.0;

  void
  reset ()
  {
    n = 0U;
    mean = 0.0;
    m2 = 0.0;
    min = 0.0;
    max = 0.0;
  }

  void
  push (double x)
  {
    if (n == 0U)
      {
        n = 1U;
        mean = x;
        m2 = 0.0;
        min = x;
        max = x;
        return;
      }

    if (x < min)
      {
        min = x;
      }
    if (x > max)
      {
        max = x;
      }

    ++n;
    double const delta = x - mean;
    mean += delta / static_cast<double> (n);
    double const delta2 = x - mean;
    m2 += delta * delta2;
  }

  double
  variance () const
  {
    return (n > 1U) ? (m2 / static_cast<double> (n - 1U)) : 0.0;
  }

  double
  stddev () const
  {
    double const v = variance ();
    return (v > 0.0) ? sqrt (v) : 0.0;
  }
};

// -----------------------------
// P^2 streaming quantile estimator (constant memory)
// Jain & Chlamtac, 1985

class P2Quantile
{
public:
  explicit P2Quantile (float p) : p_ (p) { reset (); }

  void
  reset ()
  {
    init_count_ = 0U;
    initialized_ = false;
    for (size_t i = 0; i < 5; ++i)
      {
        init_[i] = 0.0f;
        q_[i] = 0.0f;
        n_[i] = 0;
        np_[i] = 0.0f;
        dn_[i] = 0.0f;
      }
  }

  void
  push (float x)
  {
    if (!initialized_)
      {
        init_[init_count_++] = x;
        if (init_count_ == 5U)
          {
            // sort init_
            for (size_t i = 0; i < 5; ++i)
              {
                for (size_t j = i + 1; j < 5; ++j)
                  {
                    if (init_[j] < init_[i])
                      {
                        float tmp = init_[i];
                        init_[i] = init_[j];
                        init_[j] = tmp;
                      }
                  }
              }

            for (size_t i = 0; i < 5; ++i)
              {
                q_[i] = init_[i];
                n_[i] = static_cast<int32_t> (i + 1);
              }

            float const p1 = p_ / 2.0f;
            float const p2 = p_;
            float const p3 = (1.0f + p_) / 2.0f;

            np_[0] = 1.0f;
            np_[1] = 1.0f + 4.0f * p1;
            np_[2] = 1.0f + 4.0f * p2;
            np_[3] = 1.0f + 4.0f * p3;
            np_[4] = 5.0f;

            dn_[0] = 0.0f;
            dn_[1] = p1;
            dn_[2] = p2;
            dn_[3] = p3;
            dn_[4] = 1.0f;

            initialized_ = true;
          }
        return;
      }

    int32_t k;
    if (x < q_[0])
      {
        q_[0] = x;
        k = 0;
      }
    else if (x < q_[1])
      {
        k = 0;
      }
    else if (x < q_[2])
      {
        k = 1;
      }
    else if (x < q_[3])
      {
        k = 2;
      }
    else if (x <= q_[4])
      {
        k = 3;
      }
    else
      {
        q_[4] = x;
        k = 3;
      }

    for (int32_t i = k + 1; i < 5; ++i)
      {
        n_[i] += 1;
      }

    for (int32_t i = 0; i < 5; ++i)
      {
        np_[i] += dn_[i];
      }

    for (int32_t i = 1; i <= 3; ++i)
      {
        float const d = np_[i] - static_cast<float> (n_[i]);
        if ((d >= 1.0f && (n_[i + 1] - n_[i]) > 1)
            || (d <= -1.0f && (n_[i - 1] - n_[i]) < -1))
          {
            int32_t const ds = (d >= 0.0f) ? 1 : -1;

            // Parabolic prediction
            float const qip1 = q_[i + 1];
            float const qi = q_[i];
            float const qim1 = q_[i - 1];

            int32_t const nip1 = n_[i + 1];
            int32_t const ni = n_[i];
            int32_t const nim1 = n_[i - 1];

            float const a
                = static_cast<float> (ds) / static_cast<float> (nip1 - nim1);
            float const b1 = static_cast<float> (ni - nim1 + ds) * (qip1 - qi)
                             / static_cast<float> (nip1 - ni);
            float const b2 = static_cast<float> (nip1 - ni - ds) * (qi - qim1)
                             / static_cast<float> (ni - nim1);
            float const qnew = qi + a * (b1 + b2);

            // If parabolic prediction is out of bounds, use linear
            if (qnew > qim1 && qnew < qip1)
              {
                q_[i] = qnew;
              }
            else
              {
                // Linear
                q_[i] = qi
                        + static_cast<float> (ds) * (q_[i + ds] - qi)
                              / static_cast<float> (n_[i + ds] - ni);
              }

            n_[i] += ds;
          }
      }
  }

  bool
  ready () const
  {
    return initialized_;
  }

  float
  value () const
  {
    // Marker 3 (index 2) tracks the desired quantile
    return q_[2];
  }

private:
  float p_;
  uint8_t init_count_ = 0U;
  bool initialized_ = false;
  float init_[5];
  float q_[5];
  int32_t n_[5];
  float np_[5];
  float dn_[5];
};

struct Metric
{
  RunningStats stats;
  uint64_t sum_us = 0ULL;
  P2Quantile q95;
  P2Quantile q99;

  explicit Metric (bool enable_quantiles)
      : q95 (0.95f), q99 (0.99f), quantiles_enabled (enable_quantiles)
  {
  }

  void
  reset ()
  {
    stats.reset ();
    sum_us = 0ULL;
    q95.reset ();
    q99.reset ();
  }

  void
  push_u32 (uint32_t x)
  {
    stats.push (static_cast<double> (x));
    sum_us += static_cast<uint64_t> (x);
    if (quantiles_enabled)
      {
        q95.push (static_cast<float> (x));
        q99.push (static_cast<float> (x));
      }
  }

  bool
  p95_ready () const
  {
    return quantiles_enabled && q95.ready ();
  }
  bool
  p99_ready () const
  {
    return quantiles_enabled && q99.ready ();
  }

  uint32_t
  p95_u32 () const
  {
    return p95_ready () ? static_cast<uint32_t> (q95.value () + 0.5f) : 0U;
  }

  uint32_t
  p99_u32 () const
  {
    return p99_ready () ? static_cast<uint32_t> (q99.value () + 0.5f) : 0U;
  }

  uint32_t
  mean_u32 () const
  {
    return (stats.n != 0U) ? static_cast<uint32_t> (stats.mean + 0.5) : 0U;
  }

  uint32_t
  std_u32 () const
  {
    return (stats.n > 1U) ? static_cast<uint32_t> (stats.stddev () + 0.5) : 0U;
  }

  uint32_t
  min_u32 () const
  {
    return (stats.n != 0U) ? static_cast<uint32_t> (stats.min + 0.5) : 0U;
  }

  uint32_t
  max_u32 () const
  {
    return (stats.n != 0U) ? static_cast<uint32_t> (stats.max + 0.5) : 0U;
  }

  bool quantiles_enabled;
};

struct StageMetric
{
  Metric dur_us;
  uint16_t depth = 0U;
  uint32_t start_us = 0U;

  explicit StageMetric (bool quantiles) : dur_us (quantiles) {}

  void
  reset ()
  {
    dur_us.reset ();
    depth = 0U;
    start_us = 0U;
  }
};

// -----------------------------
// Global profiler state

static SessionMode s_mode = SessionMode::None;
static uint16_t s_target_hz = 0U;
static uint32_t s_runtime_ms = 0U;
static uint32_t s_period_us = 0U;

static uint32_t first_frame_start_us = 0U;
static uint32_t last_frame_end_us = 0U;
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

// Metrics
static Metric ifi_us (true);
static Metric xfer_us (true);
static Metric spi_frame_us (true);
static Metric ovh_frame_us (false);
static Metric panelset_us (true);

static Metric fetch_scope_us (true);
static uint16_t fetch_depth = 0U;
static uint32_t fetch_start_us = 0U;

static StageMetric stages[STAGE_COUNT] = {
  StageMetric (true),  // SD read: tail matters
  StageMetric (false), // decode: often deterministic
  StageMetric (false), // fill: often deterministic
  StageMetric (false), // fill all-on: deterministic
  StageMetric (false)  // stream decode: may vary; enable later if needed
};

static const char *
mode_str (SessionMode m)
{
  switch (m)
    {
    case SessionMode::Pattern:
      return "PATTERN";
    case SessionMode::Stream:
      return "STREAM";
    case SessionMode::Other:
      return "OTHER";
    case SessionMode::None:
    default:
      return "NONE";
    }
}

static inline uint32_t
hz_to_period_us (uint16_t hz)
{
  return (hz != 0U) ? (1000000UL / static_cast<uint32_t> (hz)) : 0U;
}

static inline uint32_t
pct_x100 (uint64_t sum_us, uint32_t window_us)
{
  if (window_us == 0U)
    {
      return 0U;
    }
  return static_cast<uint32_t> ((sum_us * 10000ULL)
                                / static_cast<uint64_t> (window_us));
}

static inline uint32_t
pct_count_x100 (uint32_t count, uint32_t total)
{
  if (total == 0U)
    {
      return 0U;
    }
  return static_cast<uint32_t> ((static_cast<uint64_t> (count) * 10000ULL)
                                / static_cast<uint64_t> (total));
}

static inline void
qs_user_comment (uint8_t prio, const char *s)
{
  QS_BEGIN_ID (AC::USER_COMMENT, prio)
  QS_STR (s);
  QS_END ()
}

static inline void
fmt_u32_or_na (char *dst, size_t dst_len, bool ready, uint32_t v)
{
  if (!ready)
    {
      strncpy (dst, "NA", dst_len);
      dst[dst_len - 1] = '\0';
      return;
    }
  snprintf (dst, dst_len, "%lu", static_cast<unsigned long> (v));
}

// "p99-ish" formatter: if a true p99 is available, print it; otherwise print
// the fallback (typically max). This keeps reports self-explanatory without
// requiring quantiles for every metric.
static inline void
fmt_p99ish (char *dst, size_t dst_len, bool p99_ready, uint32_t p99_val,
            uint32_t fallback_val)
{
  uint32_t const v = p99_ready ? p99_val : fallback_val;
  snprintf (dst, dst_len, "%lu", static_cast<unsigned long> (v));
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
  first_frame_start_us = 0U;
  last_frame_end_us = 0U;
  last_frame_start_us = 0U;
  current_frame_start_us = 0U;

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

  // metrics
  ifi_us.reset ();
  xfer_us.reset ();
  spi_frame_us.reset ();
  ovh_frame_us.reset ();
  panelset_us.reset ();

  fetch_scope_us.reset ();
  fetch_depth = 0U;
  fetch_start_us = 0U;

  for (uint8_t i = 0U; i < STAGE_COUNT; ++i)
    {
      stages[i].reset ();
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
  BSP::perfRefreshTickToggle ();
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

  uint32_t const start_us = micros ();
  current_frame_start_us = start_us;
  current_frame_panelset_sum_us = 0U;
  panelset_start_us = 0U;

  // Update expected period dynamically if refresh rate changes mid-session.
  if (refresh_rate_hz != 0U)
    {
      s_period_us = hz_to_period_us (refresh_rate_hz);
      s_target_hz = refresh_rate_hz;
    }

  BSP::perfFrameTransferSet (true);

  if (first_frame_start_us == 0U)
    {
      first_frame_start_us = start_us;
    }

  if (last_frame_start_us != 0U)
    {
      uint32_t const ifi = start_us - last_frame_start_us;
      ifi_us.push_u32 (ifi);
    }
  last_frame_start_us = start_us;
}

void
on_frame_end ()
{
  ++frames_completed;

  uint32_t const end_us = micros ();
  last_frame_end_us = end_us;

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

  BSP::perfFrameTransferSet (false);
}

void
on_panelset_start ()
{
  panelset_start_us = micros ();
}

void
on_panelset_end ()
{
  if (panelset_start_us == 0U)
    {
      return;
    }
  uint32_t const end_us = micros ();
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
      fetch_start_us = micros ();
      BSP::perfFetchSet (true);
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
      BSP::perfFetchSet (false);
      uint32_t const end_us = micros ();
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
      stages[idx].start_us = micros ();
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
          uint32_t const end_us = micros ();
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
            }
        }
    }
  fetch_end ();
}

// Compute derived window_us
static inline uint32_t
window_us ()
{
  if (first_frame_start_us == 0U || last_frame_end_us < first_frame_start_us)
    {
      return 0U;
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

  s.spi_frame_mean_us = spi_frame_us.mean_u32 ();
  s.spi_frame_p99_us = spi_frame_us.p99_u32 ();
  s.spi_frame_max_us = spi_frame_us.max_u32 ();

  s.ovh_frame_mean_us = ovh_frame_us.mean_u32 ();
  s.ovh_frame_max_us = ovh_frame_us.max_u32 ();

  s.panelset_n = panelset_us.stats.n;
  s.panelset_mean_us = panelset_us.mean_u32 ();
  s.panelset_p99_us = panelset_us.p99_u32 ();
  s.panelset_max_us = panelset_us.max_u32 ();

  s.sd_over_500us = sd_over_500us_count;
  s.sd_over_1000us = sd_over_1000us_count;

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
    }

  // Safe FPS estimate (pipelined model): period >= max(xfer, cpu) + guard
  // CPU estimate uses sum of per-frame stage p99/max (conservative).
  uint32_t const cpu_p99
      = (s.stage_p99_us[STAGE_SD_READ] + s.stage_p99_us[STAGE_PATTERN_DECODE]
         + s.stage_p99_us[STAGE_FILL_FRAME_BUFFER]
         + s.stage_p99_us[STAGE_FILL_ALL_ON]
         + s.stage_p99_us[STAGE_STREAM_DECODE]);
  uint32_t const cpu_max
      = (s.stage_max_us[STAGE_SD_READ] + s.stage_max_us[STAGE_PATTERN_DECODE]
         + s.stage_max_us[STAGE_FILL_FRAME_BUFFER]
         + s.stage_max_us[STAGE_FILL_ALL_ON]
         + s.stage_max_us[STAGE_STREAM_DECODE]);

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

  p.fetch_dur_mean_us = sat_u16 (fetch_scope_us.mean_u32 ());
  p.fetch_dur_max_us = sat_u16 (fetch_scope_us.max_u32 ());

  p.drop_count = sat_u16 (drops_total);
  p.defer_count = sat_u16 (s.refresh_defers);
  p.ifi_n = sat_u16 (s.ifi_n);
  p.reserved = 0U;

  return p;
}

void
format_summary (char *out, size_t out_len)
{
  Snapshot const s = compute_snapshot ();
  uint32_t const win_us = s.window_us;
  double const win_s
      = (win_us != 0U) ? (static_cast<double> (win_us) / 1.0e6) : 0.0;
  double const fps = (win_s > 0.0)
                         ? (static_cast<double> (s.frames_completed) / win_s)
                         : 0.0;

  uint32_t const drops_total = s.refresh_post_fail + s.refresh_defer_drops;

  int32_t jitter_min = 0;
  int32_t jitter_max = 0;
  if (s.ifi_n != 0U && s.period_us != 0U)
    {
      jitter_min = static_cast<int32_t> (s.ifi_min_us)
                   - static_cast<int32_t> (s.period_us);
      jitter_max = static_cast<int32_t> (s.ifi_max_us)
                   - static_cast<int32_t> (s.period_us);
    }

  char xfer_p99ish[12];
  fmt_p99ish (xfer_p99ish, sizeof (xfer_p99ish), xfer_us.p99_ready (),
              s.xfer_p99_us, s.xfer_max_us);

  char sd_p99ish[12];
  fmt_p99ish (
      sd_p99ish, sizeof (sd_p99ish), stages[STAGE_SD_READ].dur_us.p99_ready (),
      stages[STAGE_SD_READ].dur_us.p99_u32 (), s.stage_max_us[STAGE_SD_READ]);

  snprintf (out, out_len,
            "PERF_SUM hz=%u fps=%.2f IFI_mean_us=%lu std_us=%lu "
            "jitter_us=[%ld..%ld] XFER_mean_us=%lu p99ish_us=%s max_us=%lu "
            "SD_mean_us=%lu p99ish_us=%s max_us=%lu drops=%lu defers=%lu "
            "late=%lu safe_fps_p99=%u",
            static_cast<unsigned> (s.target_hz), fps,
            static_cast<unsigned long> (s.ifi_mean_us),
            static_cast<unsigned long> (s.ifi_std_us),
            static_cast<long> (jitter_min), static_cast<long> (jitter_max),
            static_cast<unsigned long> (s.xfer_mean_us), xfer_p99ish,
            static_cast<unsigned long> (s.xfer_max_us),
            static_cast<unsigned long> (s.stage_mean_us[STAGE_SD_READ]),
            sd_p99ish,
            static_cast<unsigned long> (s.stage_max_us[STAGE_SD_READ]),
            static_cast<unsigned long> (drops_total),
            static_cast<unsigned long> (s.refresh_defers),
            static_cast<unsigned long> (s.late_frames),
            static_cast<unsigned> (s.safe_fps_p99_pipe));
}

void
qs_report_session (uint8_t qs_prio, const char *reason)
{
  Snapshot const s = compute_snapshot ();
  uint32_t const win_us = s.window_us;
  uint32_t const win_ms = win_us / 1000U;

  double const win_s
      = (win_us != 0U) ? (static_cast<double> (win_us) / 1.0e6) : 0.0;
  double const fps = (win_s > 0.0)
                         ? (static_cast<double> (s.frames_completed) / win_s)
                         : 0.0;
  double const tick_hz
      = (win_s > 0.0) ? (static_cast<double> (s.refresh_ticks) / win_s) : 0.0;
  double const frames_per_tick
      = (s.refresh_ticks != 0U) ? (static_cast<double> (s.frames_completed)
                                   / static_cast<double> (s.refresh_ticks))
                                : 0.0;

  uint32_t const drops_total = s.refresh_post_fail + s.refresh_defer_drops;

  // ---- PERF_SESSION ----
  {
    char line[240];
    snprintf (line, sizeof (line),
              "PERF_SESSION reason=%s mode=%s window_ms=%lu target_hz=%u "
              "period_us=%lu runtime_ms=%lu fps=%.2f tick_hz=%.2f f/tick=%.3f "
              "frames=%lu/%lu ticks=%lu defers=%lu late=%lu drops=%lu "
              "(post=%lu defer_overflow=%lu)",
              (reason != nullptr) ? reason : "END", mode_str (s.mode),
              static_cast<unsigned long> (win_ms),
              static_cast<unsigned> (s.target_hz),
              static_cast<unsigned long> (s.period_us),
              static_cast<unsigned long> (s_runtime_ms), fps, tick_hz,
              frames_per_tick, static_cast<unsigned long> (s.frames_completed),
              static_cast<unsigned long> (s.frames_started),
              static_cast<unsigned long> (s.refresh_ticks),
              static_cast<unsigned long> (s.refresh_defers),
              static_cast<unsigned long> (s.late_frames),
              static_cast<unsigned long> (drops_total),
              static_cast<unsigned long> (s.refresh_post_fail),
              static_cast<unsigned long> (s.refresh_defer_drops));
    qs_user_comment (qs_prio, line);
  }

  // ---- PERF_TIMING ----
  {
    char p95[12];
    char p99[12];
    fmt_u32_or_na (p95, sizeof (p95), ifi_us.p95_ready (), s.ifi_p95_us);
    fmt_u32_or_na (p99, sizeof (p99), ifi_us.p99_ready (), s.ifi_p99_us);

    int32_t jitter_min = 0;
    int32_t jitter_max = 0;
    if (s.ifi_n != 0U && s.period_us != 0U)
      {
        jitter_min = static_cast<int32_t> (s.ifi_min_us)
                     - static_cast<int32_t> (s.period_us);
        jitter_max = static_cast<int32_t> (s.ifi_max_us)
                     - static_cast<int32_t> (s.period_us);
      }

    char line[240];
    snprintf (line, sizeof (line),
              "PERF_TIMING IFI_us n=%lu mean=%lu std=%lu p95=%s p99=%s "
              "min=%lu max=%lu jitter_us=[%ld..%ld]",
              static_cast<unsigned long> (s.ifi_n),
              static_cast<unsigned long> (s.ifi_mean_us),
              static_cast<unsigned long> (s.ifi_std_us), p95, p99,
              static_cast<unsigned long> (s.ifi_min_us),
              static_cast<unsigned long> (s.ifi_max_us),
              static_cast<long> (jitter_min), static_cast<long> (jitter_max));
    qs_user_comment (qs_prio, line);
  }

  // ---- PERF_XFER ----
  {
    // duties
    uint32_t const duty_xfer_x100 = pct_x100 (xfer_us.sum_us, win_us);
    uint32_t const duty_spi_x100 = pct_x100 (spi_frame_us.sum_us, win_us);

    char p99_xfer[12];
    char p99_spi[12];
    char p99_panelset[12];

    fmt_u32_or_na (p99_xfer, sizeof (p99_xfer), xfer_us.p99_ready (),
                   s.xfer_p99_us);
    fmt_u32_or_na (p99_spi, sizeof (p99_spi), spi_frame_us.p99_ready (),
                   s.spi_frame_p99_us);
    fmt_u32_or_na (p99_panelset, sizeof (p99_panelset),
                   panelset_us.p99_ready (), s.panelset_p99_us);

    uint32_t panelsets_per_frame_x10 = 0U;
    if (s.frames_completed != 0U)
      {
        panelsets_per_frame_x10 = static_cast<uint32_t> (
            (static_cast<uint64_t> (s.panelset_n) * 10ULL)
            / static_cast<uint64_t> (s.frames_completed));
      }

    char line[260];
    snprintf (
        line, sizeof (line),
        "PERF_XFER duty(xfer=%lu.%02lu%% spi=%lu.%02lu%%) "
        "xfer_us n=%lu mean=%lu p99=%s max=%lu "
        "spi_us mean=%lu p99=%s max=%lu "
        "ovh_us mean=%lu max=%lu "
        "panelset_us n=%lu mean=%lu p99=%s max=%lu panelsets/frame=%lu.%lu",
        static_cast<unsigned long> (duty_xfer_x100 / 100U),
        static_cast<unsigned long> (duty_xfer_x100 % 100U),
        static_cast<unsigned long> (duty_spi_x100 / 100U),
        static_cast<unsigned long> (duty_spi_x100 % 100U),
        static_cast<unsigned long> (s.xfer_n),
        static_cast<unsigned long> (s.xfer_mean_us), p99_xfer,
        static_cast<unsigned long> (s.xfer_max_us),
        static_cast<unsigned long> (s.spi_frame_mean_us), p99_spi,
        static_cast<unsigned long> (s.spi_frame_max_us),
        static_cast<unsigned long> (s.ovh_frame_mean_us),
        static_cast<unsigned long> (s.ovh_frame_max_us),
        static_cast<unsigned long> (s.panelset_n),
        static_cast<unsigned long> (s.panelset_mean_us), p99_panelset,
        static_cast<unsigned long> (s.panelset_max_us),
        static_cast<unsigned long> (panelsets_per_frame_x10 / 10U),
        static_cast<unsigned long> (panelsets_per_frame_x10 % 10U));

    qs_user_comment (qs_prio, line);
  }

  // ---- PERF_CPU ----
  {
    // Compute duty/total for key stages
    auto stage_name = [] (uint8_t idx) -> const char *
      {
        switch (idx)
          {
          case STAGE_SD_READ:
            return "SD";
          case STAGE_PATTERN_DECODE:
            return "DEC";
          case STAGE_FILL_FRAME_BUFFER:
            return "FILL";
          case STAGE_FILL_ALL_ON:
            return "ALLON";
          case STAGE_STREAM_DECODE:
            return "SDEC";
          default:
            return "STG";
          }
      };

    char line[300];
    size_t pos = 0U;

    pos += snprintf (line + pos, sizeof (line) - pos,
                     "PERF_CPU window_ms=%lu ",
                     static_cast<unsigned long> (win_ms));

    // Print SD/DEC/FILL always, print others if used.
    for (uint8_t idx = 0U; idx < STAGE_COUNT; ++idx)
      {
        bool const always
            = (idx == STAGE_SD_READ || idx == STAGE_PATTERN_DECODE
               || idx == STAGE_FILL_FRAME_BUFFER);
        if (!always && s.stage_n[idx] == 0U)
          {
            continue;
          }

        uint32_t const duty_x100
            = pct_x100 (stages[idx].dur_us.sum_us, win_us);
        uint32_t const total_ms
            = static_cast<uint32_t> (stages[idx].dur_us.sum_us / 1000ULL);

        char p99ish[12];
        fmt_p99ish (p99ish, sizeof (p99ish), stages[idx].dur_us.p99_ready (),
                    stages[idx].dur_us.p99_u32 (), s.stage_max_us[idx]);

        double calls_per_frame
            = (s.frames_completed != 0U)
                  ? (static_cast<double> (s.stage_n[idx])
                     / static_cast<double> (s.frames_completed))
                  : 0.0;

        pos += snprintf (
            line + pos, sizeof (line) - pos,
            "%s n=%lu c/f=%.2f mean=%lu p99ish=%s max=%lu total_ms=%lu "
            "duty=%lu.%02lu%% ",
            stage_name (idx), static_cast<unsigned long> (s.stage_n[idx]),
            calls_per_frame, static_cast<unsigned long> (s.stage_mean_us[idx]),
            p99ish, static_cast<unsigned long> (s.stage_max_us[idx]),
            static_cast<unsigned long> (total_ms),
            static_cast<unsigned long> (duty_x100 / 100U),
            static_cast<unsigned long> (duty_x100 % 100U));

        if (idx == STAGE_SD_READ)
          {
            uint32_t const over500_x100
                = pct_count_x100 (s.sd_over_500us, s.stage_n[idx]);
            uint32_t const over1000_x100
                = pct_count_x100 (s.sd_over_1000us, s.stage_n[idx]);
            pos += snprintf (
                line + pos, sizeof (line) - pos,
                "over500=%lu (%lu.%02lu%%) over1000=%lu (%lu.%02lu%%) ",
                static_cast<unsigned long> (s.sd_over_500us),
                static_cast<unsigned long> (over500_x100 / 100U),
                static_cast<unsigned long> (over500_x100 % 100U),
                static_cast<unsigned long> (s.sd_over_1000us),
                static_cast<unsigned long> (over1000_x100 / 100U),
                static_cast<unsigned long> (over1000_x100 % 100U));
          }

        if (pos >= sizeof (line))
          {
            break;
          }
      }

    qs_user_comment (qs_prio, line);
  }

  // ---- PERF_EST ----
  {
    int32_t headroom_p99 = 0;
    int32_t headroom_min = 0;
    if (s.period_us != 0U)
      {
        headroom_p99 = static_cast<int32_t> (s.period_us)
                       - static_cast<int32_t> (s.crit_p99_us);
        headroom_min = static_cast<int32_t> (s.period_us)
                       - static_cast<int32_t> (s.crit_max_us);
      }

    char line[240];
    snprintf (
        line, sizeof (line),
        "PERF_EST model=PIPE safe_fps_p99=%u safe_fps_max=%u "
        "crit_p99_us=%lu guard_p99_us=%lu crit_max_us=%lu guard_max_us=%lu "
        "limiter=%s headroom_p99_us=%ld headroom_min_us=%ld late_max_us=%lu",
        static_cast<unsigned> (s.safe_fps_p99_pipe),
        static_cast<unsigned> (s.safe_fps_max_pipe),
        static_cast<unsigned long> (s.crit_p99_us),
        static_cast<unsigned long> (s.guard_p99_us),
        static_cast<unsigned long> (s.crit_max_us),
        static_cast<unsigned long> (s.guard_max_us),
        s.limiter_is_transfer ? "TRANSFER" : "CPU",
        static_cast<long> (headroom_p99), static_cast<long> (headroom_min),
        static_cast<unsigned long> (s.max_late_us));

    qs_user_comment (qs_prio, line);
  }
}

} // namespace Perf

#endif // AC_ENABLE_PERF_PROBE
