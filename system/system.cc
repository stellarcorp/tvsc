#include "system/system.h"

namespace tvsc::system {

System::McuType& System::mcu() { return *get().mcu_; }

System::BoardType& System::board() { return *get().board_; }

System::ClockType& System::clock() { return *get().clock_; }

System::Scheduler& System::scheduler() { return get().scheduler_; }

System& System::get() {
  static System system{};
  return system;
}

}  // namespace tvsc::system
