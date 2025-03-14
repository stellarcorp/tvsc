#include <chrono>

#include "flags/flags.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"

DECLARE_uint64(run_duration_sec);

namespace tvsc::bringup {

using namespace std::chrono_literals;

template <typename ClockType, size_t QUEUE_SIZE>
tvsc::scheduler::Task<ClockType> quit(scheduler::Scheduler<ClockType, QUEUE_SIZE>& scheduler,
                                      typename ClockType::duration run_duration = 0s) {
  if (run_duration == 0s) {
    run_duration = std::chrono::seconds{FLAGS_run_duration_sec};
  }

  // Yield for the duration the script should run.
  co_yield run_duration;
  // And then stop the scheduler.
  scheduler.stop();
  co_return;
}

}  // namespace tvsc::bringup
