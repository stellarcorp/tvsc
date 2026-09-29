#pragma once

namespace tvsc::system {

enum class SyscallFn {
  NOOP,
  SLEEP,
};

enum class SyscallStatus {
  NOT_STARTED,
  WAITING,         // Waiting on an interrupt or similar event.
  DATA_AVAILABLE,  // Data is available that has not been consumed.
  COMPLETE,
};

template <typename ClockType>
class Syscall final {
 public:
  static constexpr ClockType::time_point LATER{ClockType::time_point::max()};

 private:
  SyscallFn function{SyscallFn::NOOP};

  union {
    const void* generic{nullptr};
    ClockType::time_point time_point;
  } args;
  bool has_time_point_args{false};

  void* result{nullptr};
  SyscallStatus status{SyscallStatus::NOT_STARTED};

 public:
  [[nodiscard]] constexpr ClockType::time_point estimate_data_available_at() const noexcept {
    if (has_time_point_args) {
      return args.time_point;
    } else {
      return LATER;
    }
  }

  [[nodiscard]] constexpr bool has_data_available() const noexcept {
    return status == SyscallStatus::DATA_AVAILABLE;
  }

  // Functions that actually implement the system calls. These are factory functions for the Syscall
  // structure.

  [[nodiscard]] static constexpr Syscall noop() noexcept {
    return Syscall{
        .function = SyscallFn::NOOP,
        .status = SyscallStatus::COMPLETE,
    };
  }

  [[nodiscard]] static constexpr Syscall sleep(ClockType::time_point& t) noexcept {
    return Syscall{
        .function = SyscallFn::SLEEP,
        .args.time_point = t,
        .has_time_point_arg = true,
        .status = SyscallStatus::WAITING,
    };
  }
};

}  // namespace tvsc::system
