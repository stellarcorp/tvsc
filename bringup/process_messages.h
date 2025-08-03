#pragma once

#include <chrono>

#include "scheduler/task.h"

namespace tvsc::bringup {

template <typename ClockType, typename QueueType>
tvsc::scheduler::Task<ClockType> process_messages(QueueType& queue) {
  using namespace std::chrono_literals;
  while (true) {
    queue.process_next_message();
    co_yield 50ms;
  }
}

}  // namespace tvsc::bringup
