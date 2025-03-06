#pragma once

#include "hal/interceptor.h"
#include "hal/systick/systick.h"
#include "hal/time_type.h"

namespace tvsc::hal::systick {

class SysTickInterceptor final : public Interceptor<SysTickType> {
 public:
  SysTickInterceptor(SysTickType& systick) : Interceptor(systick) {}

  TimeType current_time_micros() override;
  void increment_micros(TimeType us) override;
  void handle_interrupt() override;
};

}  // namespace tvsc::hal::systick
