#pragma once

#include <chrono>
#include <coroutine>
#include <cstdint>

namespace tvsc::system {

template <typename ClockType>
class TaskT final {
 public:
  struct promise_type;

 private:
  using HandleType = std::coroutine_handle<promise_type>;

 public:
  struct promise_type {
    // TODO(james): Replace these with a general task status that indicates what resources the task
    // is currently using, and when it might need access to the CPU again.
    ClockType::time_point wait_until_{};

    TaskT get_return_object() noexcept { return TaskT{HandleType::from_promise(*this)}; }

    std::suspend_always initial_suspend() noexcept { return {}; }

    std::suspend_always final_suspend() noexcept { return {}; }

    std::suspend_always yield_value(ClockType::time_point t) noexcept {
      wait_until_ = t;
      return {};
    }

    std::suspend_always yield_value(ClockType::duration d) noexcept {
      wait_until_ = ClockType::now() + d;
      return {};
    }

    void unhandled_exception() noexcept {}
    void return_void() noexcept {}
  };

 private:
  HandleType handle_{nullptr};

  friend class promise_type;

  TaskT(HandleType handle) noexcept : handle_(handle) {}

 public:
  TaskT() noexcept = default;
  TaskT(TaskT&& rhs) noexcept : handle_(rhs.handle_) { rhs.handle_ = nullptr; }
  ~TaskT() noexcept {
    if (handle_) {
      handle_.destroy();
    }
  }

  TaskT& operator=(TaskT&& rhs) noexcept {
    if (handle_) {
      handle_.destroy();
    }
    handle_ = rhs.handle_;
    rhs.handle_ = nullptr;
    return *this;
  }

  bool operator==(const TaskT& rhs) const noexcept { return handle_ == rhs.handle_; }

  bool is_runnable(ClockType::time_point now) const noexcept {
    if (handle_) {
      auto& promise{handle_.promise()};
      return now >= promise.wait_until_;
    } else {
      return false;
    }
  }

  ClockType::time_point estimate_runnable_at() const noexcept {
    if (handle_) {
      auto& promise{handle_.promise()};
      return promise.wait_until_;
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

}  // namespace tvsc::system
