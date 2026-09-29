#include <chrono>

#include "flags/flags.h"
#include "platforms/linker_sections.h"
#include "system/scheduler.h"
#include "system/task.h"

DECLARE_uint64(run_duration_sec);

namespace tvsc::bringup {

inline DEBUG_VAR int quit_task_stoppage{};

using namespace std::chrono_literals;

system::System::Task quit(typename system::System::ClockType::duration run_duration = 0s) {
  if (run_duration == 0s) {
    run_duration = std::chrono::seconds{FLAGS_run_duration_sec};
  }

  // Yield for the duration the script should run.
  co_yield run_duration;

  // And then stop the scheduler.
  system::System::scheduler().stop();
  quit_task_stoppage = 0xbead;

  co_return;
}

}  // namespace tvsc::bringup
