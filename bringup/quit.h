#include <chrono>

#include "flags/flags.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"

DECLARE_uint64(run_duration_sec);

namespace tvsc::bringup {

template <typename ClockType, size_t QUEUE_SIZE>
tvsc::scheduler::Task<ClockType> quit(scheduler::Scheduler<ClockType, QUEUE_SIZE>& scheduler) {
  const std::chrono::seconds run_duration{FLAGS_run_duration_sec};
  // Yield for the duration the script should run.
  co_yield run_duration;
  // And then stop the scheduler.
  scheduler.stop();
  co_return;
}

}  // namespace tvsc::bringup
