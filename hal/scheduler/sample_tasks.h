#pragma once

#include <coroutine>
#include <cstdint>

#include "gtest/gtest.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"
#include "hal/time/clock.h"
#include "hal/time/mock_clock.h"

namespace tvsc::hal::scheduler {

template <typename ClockType>
Task<ClockType> just_return() {
  co_return;
}

template <typename ClockType>
Task<ClockType> run_forever(int& run_count) {
  while (true) {
    ++run_count;
    // Return a lambda indicating that this Task is always ready to run.
    co_yield []() { return true; };
  }
}

template <typename ClockType, size_t num_iterations, uint64_t wake_interval_us>
Task<ClockType> do_something(int& run_count) {
  for (unsigned i = 0; i < num_iterations; ++i) {
    ++run_count;
    const auto wake_time{ClockType::now() + std::chrono::microseconds{wake_interval_us}};
    if (i % 2) {
      // Return a lambda indicating that this Task will be ready to run at a certain time.
      co_yield [wake_time]() -> bool { return ClockType::now() >= wake_time; };
    } else {
      // As above, but using the shorthand of just returning a uint64_t representing the time in
      // microseconds when it will be ready.
      co_yield wake_time;
    }
  }
  co_return;
}

template <typename ClockType, size_t QUEUE_SIZE, size_t SUBTASK_CREATION_RATE = 10>
Task<ClockType> creates_subtask(Scheduler<ClockType, QUEUE_SIZE>& scheduler, int& run_count) {
  while (true) {
    scheduler.add_task(run_forever<ClockType>(run_count));
    for (size_t i = 0; i < SUBTASK_CREATION_RATE; ++i) {
      ++run_count;
      // Return a lambda indicating that this Task is always ready to run.
      co_yield []() { return true; };
    }
  }
}

}  // namespace tvsc::hal::scheduler
