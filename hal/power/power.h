#pragma once

namespace tvsc::hal::power {

class Power {
 public:
  virtual ~Power() = default;

  // Note: be sure to configure an interrupt mechanism before calling this method. It does not
  // return until an interrupt fires.
  virtual void enter_stop_mode() = 0;
};

}  // namespace tvsc::hal::power
