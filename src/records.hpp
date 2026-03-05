#ifndef RECORDS_HPP
#define RECORDS_HPP

#include "qpcpp.hpp"

namespace AC
{

enum ArenaControllerRecords
{
  // User Group-0
  ETHERNET_LOG = QP::QS_USER,

  // User Group-1
  // Performance / timing probes (binary records; designed to be lightweight)
  PERF_FRAME = QP::QS_USER + 5,
  PERF_STAGE = QP::QS_USER + 6,
  PERF_DROP = QP::QS_USER + 7,
  // Explicit mode boundary markers (intended for host test harnesses).
  MODE_STARTED = QP::QS_USER + 8,
  MODE_ENDED = QP::QS_USER + 9,

  // User Group-2
  USER_COMMENT = QP::QS_USER + 10,

  // User Group-3
  DEBUG_COMMENT = QP::QS_USER + 15,

  // User Group-4
  ERROR_COMMENT = QP::QS_USER + 20,
};

} // namespace AC

#endif // RECORDS_HPP
