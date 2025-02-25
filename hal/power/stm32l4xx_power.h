#pragma once

#include "hal/power/power.h"

namespace tvsc::hal::power {

class PowerStm32L4xx final : public Power {
 public:
  void enter_sleep_mode() override;
  void enter_stop_mode() override;
};

}  // namespace tvsc::hal::power
