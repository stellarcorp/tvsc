#pragma once

namespace tvsc::hal::power {

class Power {
 public:
  virtual ~Power() = default;

  virtual void enter_low_power_run_mode() = 0;
  virtual void exit_low_power_run_mode() = 0;
};

}  // namespace tvsc::hal::power
