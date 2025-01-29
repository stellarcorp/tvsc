#pragma once

#include <array>
#include <coroutine>
#include <cstdint>
#include <string>

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
  std::array<Task, QUEUE_SIZE> task_queue_{};

  friend std::string to_string<QUEUE_SIZE>(const Scheduler&);

 public:
  Scheduler(time::Clock& clock) : clock_(&clock) {}

  size_t add_task(Task&& task) {
    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
      if (!task_queue_[i].is_valid()) {
        task_queue_[i] = std::move(task);
        return i;
      }
    }
    // TODO(james): Work out error handling strategy.
    // TODO(james): Perhaps use a vector instead of an array.
    return -1;
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

  void run_tasks_once() {
    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
      Task& task{task_queue_[i]};
      if (task.is_valid()) {
        const uint64_t current_time_us{clock_->current_time_micros()};
        if (task.is_ready(current_time_us)) {
          task.run();
        }
        if (task.is_complete()) {
          task_queue_[i] = {};
        }
      }
    }
  }

  void start() {
    while (true) {
      run_tasks_once();
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
