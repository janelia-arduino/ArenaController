#pragma once

#include <stdint.h>

// perf_port.hpp
//
// Project/platform port layer for the performance probe.
//
// This header intentionally contains only declarations. Each firmware project
// should provide a small .cpp that implements these hooks.
//
// Required:
//   - now_us32(): returns a monotonically increasing microsecond tick counter
//     (wrapping at 2^32 is OK).
//
// Optional (scope pins / logic analyzer markers):
//   - refresh_tick_toggle()
//   - frame_transfer_set(bool)
//   - fetch_set(bool)

namespace Perf
{
namespace port
{

uint32_t now_us32 ();

void refresh_tick_toggle ();
void frame_transfer_set (bool high);
void fetch_set (bool high);

} // namespace port
} // namespace Perf
