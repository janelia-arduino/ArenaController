#pragma once

#include <math.h>
#include <stddef.h>
#include <stdint.h>

// perf_core.hpp
//
// Portable, low-overhead performance measurement primitives.
//
// Design goals:
//   * No dependencies on Arduino, BSP, QP/QS, or any project-specific headers.
//   * Constant-memory streaming statistics.
//   * Optional streaming p95/p99 estimation via P^2 algorithm.
//
// This header is intended to be reusable across firmware projects.

namespace Perf
{
namespace core
{

// -----------------------------
// Running mean/stddev/min/max (Welford)

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

// Pair of p95 and p99 estimators.
struct QuantilePair
{
  P2Quantile q95{ 0.95f };
  P2Quantile q99{ 0.99f };

  void
  reset ()
  {
    q95.reset ();
    q99.reset ();
  }

  void
  push (float x)
  {
    q95.push (x);
    q99.push (x);
  }

  bool
  p95_ready () const
  {
    return q95.ready ();
  }

  bool
  p99_ready () const
  {
    return q99.ready ();
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
};

// Streaming metric accumulator. Quantile state is optional and supplied
// externally so projects can avoid paying the RAM cost when quantiles are
// disabled.
struct Metric
{
  RunningStats stats;
  uint64_t sum_us = 0ULL;
  QuantilePair *quantiles = nullptr; // nullptr when p95/p99 are disabled

  explicit Metric (QuantilePair *q = nullptr) : quantiles (q) {}

  void
  reset ()
  {
    stats.reset ();
    sum_us = 0ULL;
    if (quantiles != nullptr)
      {
        quantiles->reset ();
      }
  }

  void
  push_u32 (uint32_t x)
  {
    stats.push (static_cast<double> (x));
    sum_us += static_cast<uint64_t> (x);
    if (quantiles != nullptr)
      {
        quantiles->push (static_cast<float> (x));
      }
  }

  bool
  p95_ready () const
  {
    return (quantiles != nullptr) && quantiles->p95_ready ();
  }

  bool
  p99_ready () const
  {
    return (quantiles != nullptr) && quantiles->p99_ready ();
  }

  uint32_t
  p95_u32 () const
  {
    return p95_ready () ? quantiles->p95_u32 () : 0U;
  }

  uint32_t
  p99_u32 () const
  {
    return p99_ready () ? quantiles->p99_u32 () : 0U;
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
};

// Stage timing helper (supports nesting).
struct StageMetric
{
  Metric dur_us;
  uint16_t depth = 0U;
  uint32_t start_us = 0U;

  explicit StageMetric (QuantilePair *q = nullptr) : dur_us (q) {}

  void
  reset ()
  {
    dur_us.reset ();
    depth = 0U;
    start_us = 0U;
  }
};

// Extend a wrapping 32-bit micros counter into a monotonic 64-bit counter.
struct Micros64Extender
{
  uint64_t accum = 0ULL;
  uint32_t last = 0U;

  void
  reset ()
  {
    accum = 0ULL;
    last = 0U;
  }

  uint64_t
  extend (uint32_t now_us32)
  {
    if (last == 0U)
      {
        last = now_us32;
        accum = static_cast<uint64_t> (now_us32);
        return accum;
      }

    uint32_t const delta = now_us32 - last;
    last = now_us32;
    accum += static_cast<uint64_t> (delta);
    return accum;
  }
};

} // namespace core
} // namespace Perf
