#pragma once

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "hal/simulation/irq_generator.h"
#include "time/clockable.h"

namespace tvsc::hal::simulation {

template <typename ClockT>
class Reactor final : public time::Clockable<ClockT> {
 public:
  using ClockType = ClockT;

 private:
  struct EventTiming final {
    ClockType::time_point next_event_time;
    IrqGenerator<ClockType>* generator;
  };

  static constexpr double SCALE_FACTOR{ClockType::SCALE_FACTOR > 0 ? ClockType::SCALE_FACTOR : 1.};

  std::vector<EventTiming> timings_{};

  std::thread generation_thread_;
  std::mutex m_{};
  std::condition_variable cv_{};

  // Synchronization mechanism to block the core thread until an IRQ wakes it up. The thread
  // simulating the core blocks when the core is put in either sleep or stop mode. This semaphore is
  // how it blocks when running under simulation.
  std::mutex core_thread_mutex_{};
  std::condition_variable core_thread_block_{};

  bool stop_requested_{false};

  void reorder_timings() noexcept {
    std::sort(timings_.begin(), timings_.end(), [](const EventTiming& lhs, const EventTiming& rhs) {
      return lhs.next_event_time <= rhs.next_event_time;
    });
  }

  void generate_irqs_once() noexcept {
    auto current_time{ClockType::now()};
    for (auto& timing : timings_) {
      if (timing.next_event_time <= current_time) {
        wake_core_thread();
        timing.generator->generate_interrupt(current_time);
        current_time = ClockType::now();
        timing.next_event_time =
            current_time + timing.generator->next_interrupt_in(current_time) / SCALE_FACTOR;
      } else {
        break;
      }
    }

    reorder_timings();
  }

  void generate_irqs() noexcept {
    using namespace std::chrono_literals;
    std::unique_lock lock(m_);
    while (!stop_requested_) {
      auto current_time{ClockType::now()};
      auto wait_time{current_time + 10ms / SCALE_FACTOR};
      if (!timings_.empty()) {
        wait_time = std::min(wait_time, timings_[0].next_event_time);
      }
      cv_.wait_until(lock, wait_time);
      generate_irqs_once();
    }
  }

  [[nodiscard]] virtual ClockType::time_point update_time(
      ClockType::time_point current_time) noexcept override {
    std::lock_guard lock(m_);
    if (!timings_.empty()) {
      if (timings_[0].next_event_time < current_time) {
        current_time = timings_[0].next_event_time;
      }
    }
    return current_time;
  }

  virtual void run(ClockType::time_point current_time) noexcept override {
    // Signal the generation thread in order to generate appropriate IRQs for the current time.
    cv_.notify_all();
  }

  void wake_core_thread() noexcept {
    // Notify the core thread that it can resume.
    core_thread_block_.notify_all();
  }

 public:
  Reactor(ClockType& clock) noexcept
      : time::Clockable<ClockType>(clock), generation_thread_(&Reactor::generate_irqs, this) {}

  ~Reactor() noexcept {
    if (generation_thread_.joinable()) {
      // Tell the generation_thread to stop.
      {
        std::lock_guard lock{m_};
        stop_requested_ = true;
      }

      // Wake-up the generation thread so that it can exit.
      cv_.notify_all();

      // Allow the Reactor to be destroyed only after the generation_thread has finished.
      generation_thread_.join();
    }
  }

  void block_core_thread_until_irq() noexcept {
    // Block until an IRQ is about to be generated.
    std::unique_lock lock{core_thread_mutex_};
    core_thread_block_.wait(lock);
  }

  void add_generator(IrqGenerator<ClockType>& generator) noexcept {
    const auto current_time{ClockType::now()};
    auto next_event_time = current_time + generator.next_interrupt_in(current_time) / SCALE_FACTOR;

    {
      std::lock_guard lock(m_);
      timings_.emplace_back(std::move(next_event_time), &generator);
      reorder_timings();
    }

    // Wake the generation thread as the new generator may have changed the timings.
    // Note that we could also see if the new generator needs attention before the others by
    // checking if it is at the front of the queue. The approach here is the lazy approach, but it
    // should not matter as we don't anticipate adding generators often once the simulation is
    // running.
    cv_.notify_all();
  }
};

}  // namespace tvsc::hal::simulation
