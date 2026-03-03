#ifndef MODE_TRACE_HPP
#define MODE_TRACE_HPP

#include <stdint.h>

namespace AC
{
namespace ModeTrace
{

// Stable IDs for mode-centric performance sessions.
//
// These IDs are intended to be used in QS records and/or host-side tests.
// Keep them stable once tests depend on them.
enum class ModeId : uint8_t
{
  AllOff = 0U,
  AllOn = 1U,
  StreamingFrame = 2U,
  PlayingPattern = 3U,
  ShowingPatternFrame = 4U,
  AnalogClosedLoop = 5U,
};

enum class EndReason : uint8_t
{
  Stopped = 0U,     // Explicit ALL_OFF / stop command
  Completed = 1U,   // Natural completion (e.g. runtime finished)
  Error = 2U,       // Fatal error (card/pattern/etc)
  Interrupted = 3U, // Mode changed by another command
};

static inline const char *
mode_name (ModeId m)
{
  switch (m)
    {
    case ModeId::AllOff:
      return "ALL_OFF";
    case ModeId::AllOn:
      return "ALL_ON";
    case ModeId::StreamingFrame:
      return "STREAM_FRAME";
    case ModeId::PlayingPattern:
      return "PLAY_PATTERN";
    case ModeId::ShowingPatternFrame:
      return "SHOW_PATTERN_FRAME";
    case ModeId::AnalogClosedLoop:
      return "ANALOG_CLOSED_LOOP";
    default:
      return "UNKNOWN";
    }
}

static inline const char *
reason_name (EndReason r)
{
  switch (r)
    {
    case EndReason::Stopped:
      return "STOPPED";
    case EndReason::Completed:
      return "COMPLETED";
    case EndReason::Error:
      return "ERROR";
    case EndReason::Interrupted:
      return "INTERRUPTED";
    default:
      return "UNKNOWN";
    }
}

} // namespace ModeTrace
} // namespace AC

#endif // MODE_TRACE_HPP
