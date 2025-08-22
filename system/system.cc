#include "system/system.h"

namespace tvsc::system {

System::BoardType& System::board() { return get().board_; }

System::ClockType& System::clock() { return get().clock_; }

System::SchedulerType& System::scheduler() { return get().scheduler_; }

System& System::get() {
  static System system{};
  return system;
}

}  // namespace tvsc::system
