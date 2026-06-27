#pragma once

#include <cstdint>

#include "hal/board/board.h"
#include "hal/mcu/mcu.h"
#include "system/scheduler.h"
#include "system/task.h"
#include "time/embedded_clock.h"

namespace tvsc::system {

class System final {
 public:
#if defined(CONFIG_SCHEDULER_QUEUE_SIZE)
  static constexpr size_t SCHEDULER_QUEUE_SIZE{CONFIG_SCHEDULER_QUEUE_SIZE};
#else
  static constexpr size_t SCHEDULER_QUEUE_SIZE{5};
#endif

  using ClockType = tvsc::time::EmbeddedClock;
  using McuType = tvsc::hal::mcu::Mcu;
  using BoardType = tvsc::hal::board::Board;
  using Scheduler = SchedulerT<ClockType, SCHEDULER_QUEUE_SIZE>;
  using Task = TaskT<ClockType>;

 private:
  McuType* const mcu_{&McuType::mcu()};
  BoardType* const board_{&BoardType::board()};
  ClockType* const clock_{&ClockType::clock()};

  Scheduler scheduler_{mcu_->rcc()};

  // Singleton with instance held in static accessor function.
  System() = default;

 public:
  static ClockType& clock();
  static McuType& mcu();
  static BoardType& board();
  static Scheduler& scheduler();

  static System& get();
};

}  // namespace tvsc::system
