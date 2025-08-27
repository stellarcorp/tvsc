#pragma once

#include <chrono>

#include "system/system.h"

namespace tvsc::bringup {

template <typename QueueType>
tvsc::system::System::Task process_messages(QueueType& queue) {
  using namespace std::chrono_literals;
  while (true) {
    queue.process_next_message();
    co_yield 50ms;
  }
}

}  // namespace tvsc::bringup
