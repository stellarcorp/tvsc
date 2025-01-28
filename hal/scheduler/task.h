#pragma once

#include <coroutine>
#include <cstdint>
#include <functional>

namespace tvsc::hal::scheduler {

class Task final {
 public:
  struct promise_type;

 private:
  using HandleType = std::coroutine_handle<promise_type>;

 public:
  struct promise_type {
    uint64_t wait_until_us_{};
    std::function<bool()> ready_condition_{};

    Task get_return_object() noexcept { return Task{HandleType::from_promise(*this)}; }

    std::suspend_always initial_suspend() noexcept { return {}; }

    std::suspend_always final_suspend() noexcept { return {}; }

    std::suspend_always yield_value(uint64_t t) noexcept {
      wait_until_us_ = t;
      ready_condition_ = {};
      return {};
    }

    std::suspend_always yield_value(std::function<bool()> ready_condition) noexcept {
      ready_condition_ = ready_condition;
      return {};
    }

    void unhandled_exception() noexcept {}
    void return_void() noexcept {}
  };

 private:
  HandleType handle_;

  friend class promise_type;

  Task(HandleType handle) : handle_(handle) {}

 public:
  ~Task() { handle_.destroy(); }

  bool is_ready(uint64_t now_us) noexcept {
    auto& promise{handle_.promise()};
    if (promise.ready_condition_) {
      return promise.ready_condition_();
    } else {
      return now_us >= promise.wait_until_us_;
    }
  }

  bool is_complete() noexcept { return handle_.done(); }

  void run() noexcept { handle_(); }
};

}  // namespace tvsc::hal::scheduler
