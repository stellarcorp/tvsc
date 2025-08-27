#pragma once

#include <chrono>
#include <coroutine>
#include <cstdint>

#include "gtest/gtest.h"
#include "system/scheduler.h"
#include "system/task.h"
#include "time/mock_clock.h"

namespace tvsc::system {

template <typename ClockType>
TaskT<ClockType> just_return() {
  co_return;
}

template <typename ClockType>
TaskT<ClockType> run_forever(int& run_count) {
  using namespace std::chrono_literals;
  while (true) {
    ++run_count;
    // Return a lambda indicating that this Task is always ready to run.
    co_yield 0ms;
  }
}

template <typename ClockType, size_t num_iterations, uint64_t wake_interval_us>
TaskT<ClockType> do_something(int& run_count) {
  for (unsigned i = 0; i < num_iterations; ++i) {
    ++run_count;
    const auto wake_time{ClockType::now() + std::chrono::microseconds{wake_interval_us}};
    co_yield wake_time;
  }
  co_return;
}

template <typename ClockType, size_t QUEUE_SIZE, size_t SUBTASK_CREATION_RATE = 10>
TaskT<ClockType> creates_subtask(SchedulerT<ClockType, QUEUE_SIZE>& scheduler, int& run_count) {
  using namespace std::chrono_literals;
  while (true) {
    scheduler.add_task(run_forever<ClockType>(run_count));
    for (size_t i = 0; i < SUBTASK_CREATION_RATE; ++i) {
      ++run_count;
      co_yield 0ms;
    }
  }
}

}  // namespace tvsc::system
