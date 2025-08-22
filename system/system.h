#pragma once

#include <cstdint>

#include "hal/board/board.h"
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
  using BoardType = tvsc::hal::board::Board;
  using SchedulerType = Scheduler<ClockType, SCHEDULER_QUEUE_SIZE>;
  using TaskType = Task<ClockType>;

 private:
  BoardType& board_{BoardType::board()};
  ClockType& clock_{ClockType::clock()};

  SchedulerType scheduler_{board_.rcc()};

  // Singleton with instance held in static accessor function.
  System() = default;

 public:
  static ClockType& clock();
  static BoardType& board();
  static SchedulerType& scheduler();

  static System& get();
};

}  // namespace tvsc::system
