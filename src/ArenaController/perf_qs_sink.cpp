#include "perf.hpp"

#if AC_ENABLE_PERF_PROBE

#include <stdio.h>
#include <string.h>

#include "qpcpp.hpp"
#include "records.hpp"

using namespace QP;

namespace Perf
{

// Stage labels / formatting policy come from the project spec list
// (perf_spec.hpp). Keeping them in one place avoids updating multiple
// switch statements when stages change.
static const char *const kStageShortName[STAGE_COUNT] = {
#define AC_PERF_STAGE_NAME(_id, _label, _always, _quant) _label,
  AC_PERF_STAGES (AC_PERF_STAGE_NAME)
#undef AC_PERF_STAGE_NAME
};

static const bool kStageAlwaysPrint[STAGE_COUNT] = {
#define AC_PERF_STAGE_ALWAYS(_id, _label, _always, _quant) ((_always) != 0),
  AC_PERF_STAGES (AC_PERF_STAGE_ALWAYS)
#undef AC_PERF_STAGE_ALWAYS
};

static const char *const kUpdateShortName[UPD_COUNT] = {
#define AC_PERF_UPD_NAME(_id, _label) _label,
  AC_PERF_UPDATES (AC_PERF_UPD_NAME)
#undef AC_PERF_UPD_NAME
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
pct_x100 (uint64_t sum_us, uint64_t window_us)
{
  if (window_us == 0ULL)
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
// the fallback (typically max).
static inline void
fmt_p99ish (char *dst, size_t dst_len, bool p99_ready, uint32_t p99_val,
            uint32_t fallback_val)
{
  uint32_t const v = p99_ready ? p99_val : fallback_val;
  snprintf (dst, dst_len, "%lu", static_cast<unsigned long> (v));
}

void
format_summary (char *out, size_t out_len)
{
  Snapshot const s = snapshot ();
  uint64_t const win_us = s.window_us;
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

  bool const xfer_p99_ready = (s.xfer_n >= 5U);
  char xfer_p99ish[12];
  fmt_p99ish (xfer_p99ish, sizeof (xfer_p99ish), xfer_p99_ready, s.xfer_p99_us,
              s.xfer_max_us);

  // stage_p99_us is already "p99-ish" (p99 if available else max)
  char sd_p99ish[12];
  snprintf (sd_p99ish, sizeof (sd_p99ish), "%lu",
            static_cast<unsigned long> (s.stage_p99_us[STAGE_SD_READ]));

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
  Snapshot const s = snapshot ();
  uint64_t const win_us = s.window_us;
  uint32_t const win_ms = static_cast<uint32_t> (win_us / 1000ULL);

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
              static_cast<unsigned long> (s.runtime_ms), fps, tick_hz,
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
    bool const p95_ready = (s.ifi_n >= 5U);
    bool const p99_ready = (s.ifi_n >= 5U);

    char p95[12];
    char p99[12];
    fmt_u32_or_na (p95, sizeof (p95), p95_ready, s.ifi_p95_us);
    fmt_u32_or_na (p99, sizeof (p99), p99_ready, s.ifi_p99_us);

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
    uint32_t const duty_xfer_x100 = pct_x100 (s.xfer_sum_us, win_us);
    uint32_t const duty_spi_x100 = pct_x100 (s.spi_frame_sum_us, win_us);

    bool const xfer_p99_ready = (s.xfer_n >= 5U);
    bool const spi_p99_ready = (s.xfer_n >= 5U); // spi_frame is per-frame
    bool const panelset_p99_ready = (s.panelset_n >= 5U);

    char p99_xfer[12];
    char p99_spi[12];
    char p99_panelset[12];

    fmt_u32_or_na (p99_xfer, sizeof (p99_xfer), xfer_p99_ready, s.xfer_p99_us);
    fmt_u32_or_na (p99_spi, sizeof (p99_spi), spi_p99_ready,
                   s.spi_frame_p99_us);
    fmt_u32_or_na (p99_panelset, sizeof (p99_panelset), panelset_p99_ready,
                   s.panelset_p99_us);

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
    char line[512];
    size_t pos = 0U;

    pos += snprintf (line + pos, sizeof (line) - pos,
                     "PERF_CPU window_ms=%lu ",
                     static_cast<unsigned long> (win_ms));

    // Print SD/DEC/FILL always, print others if used.
    for (uint8_t idx = 0U; idx < STAGE_COUNT; ++idx)
      {
        if (!kStageAlwaysPrint[idx] && s.stage_n[idx] == 0U)
          {
            continue;
          }

        uint32_t const duty_x100 = pct_x100 (s.stage_sum_us[idx], win_us);
        uint32_t const total_ms
            = static_cast<uint32_t> (s.stage_sum_us[idx] / 1000ULL);

        // stage_p99_us is already "p99-ish"
        char p99ish[12];
        snprintf (p99ish, sizeof (p99ish), "%lu",
                  static_cast<unsigned long> (s.stage_p99_us[idx]));

        double calls_per_frame
            = (s.frames_completed != 0U)
                  ? (static_cast<double> (s.stage_n[idx])
                     / static_cast<double> (s.frames_completed))
                  : 0.0;

        pos += snprintf (
            line + pos, sizeof (line) - pos,
            "%s n=%lu c/f=%.2f mean=%lu p99ish=%s max=%lu total_ms=%lu "
            "duty=%lu.%02lu%% ",
            kStageShortName[idx], static_cast<unsigned long> (s.stage_n[idx]),
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
            uint32_t const over2000_x100
                = pct_count_x100 (s.sd_over_2000us, s.stage_n[idx]);
            uint32_t const over5000_x100
                = pct_count_x100 (s.sd_over_5000us, s.stage_n[idx]);
            pos += snprintf (
                line + pos, sizeof (line) - pos,
                "over500=%lu (%lu.%02lu%%) over1000=%lu (%lu.%02lu%%) "
                "over2000=%lu (%lu.%02lu%%) over5000=%lu (%lu.%02lu%%) ",
                static_cast<unsigned long> (s.sd_over_500us),
                static_cast<unsigned long> (over500_x100 / 100U),
                static_cast<unsigned long> (over500_x100 % 100U),
                static_cast<unsigned long> (s.sd_over_1000us),
                static_cast<unsigned long> (over1000_x100 / 100U),
                static_cast<unsigned long> (over1000_x100 % 100U),
                static_cast<unsigned long> (s.sd_over_2000us),
                static_cast<unsigned long> (over2000_x100 / 100U),
                static_cast<unsigned long> (over2000_x100 % 100U),
                static_cast<unsigned long> (s.sd_over_5000us),
                static_cast<unsigned long> (over5000_x100 / 100U),
                static_cast<unsigned long> (over5000_x100 % 100U));
          }

        if (pos >= sizeof (line))
          {
            break;
          }
      }

    qs_user_comment (qs_prio, line);
  }

  // ---- PERF_UPD ----
  {
    for (uint8_t idx = 0U; idx < UPD_COUNT; ++idx)
      {
        Snapshot::UpdateSnapshot const &u = s.upd[idx];
        if (u.received == 0U && u.processed == 0U && u.applied == 0U)
          {
            continue;
          }

        bool const ifi_p99_ready = (u.ifi_n >= 5U);
        bool const lat_p99_ready = (u.latency_n >= 5U);

        char ifi_p99[12];
        char lat_p99[12];
        fmt_u32_or_na (ifi_p99, sizeof (ifi_p99), ifi_p99_ready, u.ifi_p99_us);
        fmt_u32_or_na (lat_p99, sizeof (lat_p99), lat_p99_ready,
                       u.latency_p99_us);

        char line[260];
        snprintf (
            line, sizeof (line),
            "PERF_UPD kind=%s recv=%lu proc=%lu commit=%lu applied=%lu "
            "coalesced=%lu "
            "ifi_us n=%lu mean=%lu p99=%s max=%lu "
            "latency_us n=%lu mean=%lu p99=%s max=%lu",
            kUpdateShortName[idx], static_cast<unsigned long> (u.received),
            static_cast<unsigned long> (u.processed),
            static_cast<unsigned long> (u.committed),
            static_cast<unsigned long> (u.applied),
            static_cast<unsigned long> (u.coalesced),
            static_cast<unsigned long> (u.ifi_n),
            static_cast<unsigned long> (u.ifi_mean_us), ifi_p99,
            static_cast<unsigned long> (u.ifi_max_us),
            static_cast<unsigned long> (u.latency_n),
            static_cast<unsigned long> (u.latency_mean_us), lat_p99,
            static_cast<unsigned long> (u.latency_max_us));
        qs_user_comment (qs_prio, line);
      }
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
