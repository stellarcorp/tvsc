#pragma once

#include <array>
#include <chrono>
#include <coroutine>
#include <cstdint>
#include <string>

#include "hal/error.h"
#include "hal/rcc/rcc.h"
#include "scheduler/task.h"

namespace tvsc::scheduler {

template <typename ClockType, size_t QUEUE_SIZE>
class Scheduler;

template <typename ClockType, size_t QUEUE_SIZE>
std::string to_string(const Scheduler<ClockType, QUEUE_SIZE>& scheduler);

template <typename ClockT, size_t QUEUE_SIZE>
class Scheduler final {
 public:
  using ClockType = ClockT;
  using TaskType = Task<ClockType>;

 private:
  ClockType* clock_{&ClockType::clock()};
  tvsc::hal::rcc::Rcc* rcc_;
  std::array<TaskType, QUEUE_SIZE> task_queue_{};
  bool stop_requested_{false};

  friend std::string to_string<ClockType, QUEUE_SIZE>(const Scheduler&);

 public:
  Scheduler(tvsc::hal::rcc::Rcc& rcc) : rcc_(&rcc) {}

  size_t add_task(TaskType&& task) {
    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
      if (!task_queue_[i].is_valid()) {
        task_queue_[i] = std::move(task);
        return i;
      }
    }
    // No more room for tasks. The size of the scheduler is fixed at compile-time and is required to
    // have enough space for all possible tasks.
    error();
  }

  void remove_task(size_t index) { task_queue_.at(index) = {}; }

  TaskType& task(size_t index) noexcept { return task_queue_.at(index); }
  const TaskType& task(size_t index) const noexcept { return task_queue_.at(index); }

  size_t queue_size() const {
    size_t size{};
    for (auto& task : task_queue_) {
      if (task.is_valid()) {
        ++size;
      }
    }
    return size;
  }

  auto run_tasks_once() {
    using namespace std::chrono_literals;
    auto next_wakeup_time{clock_->current_time() + 5s};
    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
      TaskType& task{task_queue_[i]};
      if (task.is_valid()) {
        if (task.is_ready(clock_->current_time())) {
          task.run();
        }
        if (task.is_complete()) {
          task_queue_[i] = {};
        }
        next_wakeup_time = std::min(next_wakeup_time, task.estimate_ready_at());
      }
    }
    return next_wakeup_time;
  }

  void start() {
    // TODO(james): Play around with this strategy. Currently, this strategy assumes that we have a
    // CPU-heavy workload. This assumptions is likely wrong. Bus transfers (I2C, CAN bus, and SPI)
    // probably won't need max speed but may need frequent (small number of microseconds) CPU
    // access. See https://chatgpt.com/share/67b593a9-5fb4-8006-9cdf-1d22aa22c574 for ideas as well
    // as timing estimates for switching clock speeds and entering/exiting stop mode.
    //
    // This particular strategy is puts the clock at an energy efficient speed and sleeps (by
    // putting CPU in stop mode) when tasks aren't ready.
    //
    // We use this strategy because:
    // - It is simple.
    // - The latency to switch clock speeds is ~500 us.
    // - The latency to configure a timer, enter stop mode, and exit stop mode is also ~500 us.
    // That is, switching clock speeds here appears to be a false savings; we could enter stop mode
    // in the same time, and stop mode uses vastly less power.
    rcc_->set_clock_to_energy_efficient_speed();
    while (!stop_requested_) {
      auto next_wakeup_time{run_tasks_once()};
      clock_->sleep(next_wakeup_time);
    }
  }

  void stop() { stop_requested_ = true; }
};

template <typename ClockType, size_t QUEUE_SIZE>
std::string to_string(const Scheduler<ClockType, QUEUE_SIZE>& scheduler) {
  using std::to_string;

  std::string result{};
  result.append("[\n");
  for (size_t i = 0; i < QUEUE_SIZE; ++i) {
    result.append("  ")
        .append(to_string(reinterpret_cast<uint64_t>(&scheduler.task_queue_[i])))
        .append("\n");
  }
  result.append("]\n");
  return result;
}

}  // namespace tvsc::scheduler
