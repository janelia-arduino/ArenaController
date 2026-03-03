#ifndef RECORDS_HPP
#define RECORDS_HPP

#include "qpcpp.hpp"

namespace AC
{

enum ArenaControllerRecords
{
  ETHERNET_LOG = QP::QS_USER,
  // Performance / timing probes (binary records; designed to be lightweight)
  PERF_FRAME = QP::QS_USER + 1,
  PERF_STAGE = QP::QS_USER + 2,
  PERF_DROP = QP::QS_USER + 3,
  // Explicit mode boundary markers (intended for host test harnesses).
  MODE_STARTED = QP::QS_USER + 4,
  USER_COMMENT = QP::QS_USER + 5,
  MODE_ENDED = QP::QS_USER + 6,
};

} // namespace AC

#endif // RECORDS_HPP
