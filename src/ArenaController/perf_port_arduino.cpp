#include "perf/perf_port.hpp"

#include "constants.hpp"

#if AC_ENABLE_PERF_PROBE

#include <Arduino.h>

#include "bsp.hpp"

namespace Perf
{
namespace port
{

uint32_t
now_us32 ()
{
  return micros ();
}

void
refresh_tick_toggle ()
{
  BSP::perfRefreshTickToggle ();
}

void
frame_transfer_set (bool high)
{
  BSP::perfFrameTransferSet (high);
}

void
fetch_set (bool high)
{
  BSP::perfFetchSet (high);
}

} // namespace port
} // namespace Perf

#else

// If perf probe is disabled, still provide symbols to satisfy the linker.
namespace Perf
{
namespace port
{

uint32_t
now_us32 ()
{
  return 0U;
}

void
refresh_tick_toggle ()
{
}

void
frame_transfer_set (bool)
{
}

void
fetch_set (bool)
{
}

} // namespace port
} // namespace Perf

#endif // AC_ENABLE_PERF_PROBE
