#pragma once

#include <array>
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
  std::array<Task*, QUEUE_SIZE> task_queue_{};
  Task* current_task_{nullptr};

  friend std::string to_string<QUEUE_SIZE>(const Scheduler&);

 public:
  Scheduler(time::Clock& clock) : clock_(&clock) {}

  void add_task(Task& task) {
    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
      if (task_queue_[i] == nullptr) {
        task_queue_[i] = &task;
        break;
      }
    }
  }

  void remove_task(Task& task) {
    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
      if (task_queue_[i] == &task) {
        task_queue_[i] = nullptr;
        break;
      }
    }
  }

  size_t queue_size() const {
    size_t size{};
    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
      if (task_queue_[i] != nullptr) {
        ++size;
      }
    }
    return size;
  }

  void run_tasks_once() {
    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
      Task* task{task_queue_[i]};
      if (task != nullptr) {
        const uint64_t current_time_us{clock_->current_time_micros()};
        current_task_ = task;
        if (task->is_ready(current_time_us)) {
          task->run();
        }
        if (task->is_complete()) {
          task_queue_[i] = nullptr;
        }
        current_task_ = nullptr;
      }
    }
  }

  void start() {
    while (true) {
      run_tasks_once();
    }
  }

  time::Clock& clock() { return *clock_; }

  Task* current_task() { return current_task_; }
};

template <size_t QUEUE_SIZE>
std::string to_string(const Scheduler<QUEUE_SIZE>& scheduler) {
  using std::to_string;

  std::string result{};
  result.append("[\n");
  for (size_t i = 0; i < QUEUE_SIZE; ++i) {
    result.append("  ")
        .append(to_string(reinterpret_cast<uint64_t>(scheduler.task_queue_[i])))
        .append("\n");
  }
  result.append("]\n");
  return result;
}

}  // namespace tvsc::hal::scheduler
