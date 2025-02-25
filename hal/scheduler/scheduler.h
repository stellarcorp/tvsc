#pragma once

#include <array>
#include <chrono>
#include <coroutine>
#include <cstdint>
#include <string>

#include "hal/error.h"
#include "hal/rcc/rcc.h"
#include "hal/scheduler/task.h"
#include "hal/time/clock.h"

namespace tvsc::hal::scheduler {

template <size_t QUEUE_SIZE>
class Scheduler;

template <size_t QUEUE_SIZE>
std::string to_string(const Scheduler<QUEUE_SIZE>& scheduler);

template <size_t QUEUE_SIZE>
class Scheduler final {
 private:
  time::Clock* clock_;
  rcc::Rcc* rcc_;
  std::array<Task, QUEUE_SIZE> task_queue_{};

  friend std::string to_string<QUEUE_SIZE>(const Scheduler&);

 public:
  Scheduler(time::Clock& clock, rcc::Rcc& rcc) : clock_(&clock), rcc_(&rcc) {}

  size_t add_task(Task&& task) {
    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
      if (!task_queue_[i].is_valid()) {
        task_queue_[i] = std::move(task);
        task_queue_[i].set_clock(*clock_);
        return i;
      }
    }
    // No more room for tasks. The size of the scheduler is fixed at compile-time and is required to
    // have enough space for all possible tasks.
    error();
  }

  void remove_task(size_t index) { task_queue_.at(index) = {}; }

  Task& task(size_t index) noexcept { return task_queue_.at(index); }
  const Task& task(size_t index) const noexcept { return task_queue_.at(index); }

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
      Task& task{task_queue_[i]};
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

  [[noreturn]] void start() {
    // TODO(james): Play around with this strategy. Currently, this strategy assumes that we have a
    // CPU-heavy workload and that we can ignore flash wait states. Both of these assumptions are
    // likely wrong. Bus transfers (I2C, CAN bus, and SPI) probably won't need max speed but may
    // need frequent (small number of microseconds) CPU access. See
    // https://chatgpt.com/share/67b593a9-5fb4-8006-9cdf-1d22aa22c574 for ideas as well as timing
    // estimates for switching clock speeds and entering/exiting stop mode.
    //
    // This particular strategy is puts the clock at an energy efficient speed and sleeps when tasks
    // aren't ready.
    //
    // We use this strategy because:
    // - It is simple.
    // - The latency to switch clock speeds is ~500 us.
    // - The latency to configure a timer, enter stop mode, and exit stop mode is also ~500 us.
    // - The latency to set up a timer, enter sleep mode, and exit sleep mode is about 25 us.
    // That is, switching clock speeds here appears to be a false savings; we could enter stop mode
    // in the same time, and stop mode uses vastly less power.
    rcc_->set_clock_to_energy_efficient_speed();
    while (true) {
      auto next_wakeup_time{run_tasks_once()};
      clock_->sleep(next_wakeup_time);
    }
  }

  time::Clock& clock() { return *clock_; }
};

template <size_t QUEUE_SIZE>
std::string to_string(const Scheduler<QUEUE_SIZE>& scheduler) {
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

}  // namespace tvsc::hal::scheduler
