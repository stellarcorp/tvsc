#pragma once

#include "hal/interceptor.h"
#include "hal/rcc/rcc.h"

namespace tvsc::hal::rcc {

class RccInterceptor final : public Interceptor<Rcc> {
 public:
  RccInterceptor(Rcc& rcc) : Interceptor(rcc) {}

  void set_clock_to_max_speed() override;
  void set_clock_to_min_speed() override;
  void set_clock_to_energy_efficient_speed() override;
  void restore_clock_speed() override;
};

}  // namespace tvsc::hal::rcc
