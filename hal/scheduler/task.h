#pragma once

#include <chrono>
#include <coroutine>
#include <cstdint>
#include <functional>

#include "hal/time/clock.h"

namespace tvsc::hal::scheduler {

template <typename ClockType>
class Task final {
 public:
  struct promise_type;

 private:
  using HandleType = std::coroutine_handle<promise_type>;

 public:
  struct promise_type {
    // TODO(james): Replace these with a general task status that indicates what resources the task
    // is currently using, and when it might need access to the CPU again.
    ClockType::time_point wait_until_{};
    std::function<bool()> ready_condition_{};

    Task get_return_object() noexcept { return Task{HandleType::from_promise(*this)}; }

    std::suspend_always initial_suspend() noexcept { return {}; }

    std::suspend_always final_suspend() noexcept { return {}; }

    std::suspend_always yield_value(ClockType::time_point t) noexcept {
      wait_until_ = t;
      ready_condition_ = {};
      return {};
    }

    std::suspend_always yield_value(ClockType::duration d) noexcept {
      wait_until_ = ClockType::now() + d;
      ready_condition_ = {};
      return {};
    }

    std::suspend_always yield_value(std::function<bool()> ready_condition) noexcept {
      wait_until_ = {};
      ready_condition_ = ready_condition;
      return {};
    }

    void unhandled_exception() noexcept {}
    void return_void() noexcept {}
  };

 private:
  HandleType handle_{nullptr};

  friend class promise_type;

  Task(HandleType handle) noexcept : handle_(handle) {}

 public:
  Task() noexcept = default;
  Task(Task&& rhs) noexcept : handle_(rhs.handle_) { rhs.handle_ = nullptr; }
  ~Task() noexcept {
    if (handle_) {
      handle_.destroy();
    }
  }

  Task& operator=(Task&& rhs) noexcept {
    if (handle_) {
      handle_.destroy();
    }
    handle_ = rhs.handle_;
    rhs.handle_ = nullptr;
    return *this;
  }

  bool operator==(const Task& rhs) const noexcept { return handle_ == rhs.handle_; }

  bool is_ready(ClockType::time_point now) const noexcept {
    if (handle_) {
      auto& promise{handle_.promise()};
      if (promise.ready_condition_) {
        return promise.ready_condition_();
      } else {
        return now >= promise.wait_until_;
      }
    } else {
      return false;
    }
  }

  ClockType::time_point estimate_ready_at() const noexcept {
    if (handle_) {
      auto& promise{handle_.promise()};
      if (promise.ready_condition_) {
        return ClockType::time_point::max();
      } else {
        return promise.wait_until_;
      }
    } else {
      return ClockType::time_point::max();
    }
  }

  bool is_complete() const noexcept {
    if (handle_) {
      return handle_.done();
    } else {
      return true;
    }
  }

  bool is_valid() const noexcept { return bool(handle_); }

  void run() noexcept { handle_(); }
};

}  // namespace tvsc::hal::scheduler
