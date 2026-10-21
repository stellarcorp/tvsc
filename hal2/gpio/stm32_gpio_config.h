#pragma once

#include <stdexcept>

#include "base/except.h"
#include "hal2/config/facility.h"
#include "hal2/config/system_builder.h"
#include "hal2/config/vector_table.h"
#include "hal2/gpio/gpio.h"
#include "hal2/gpio/stm32_gpio.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::gpio {

template <PortNumber PORT, typename System>
  requires(PORT < 8)
class Stm32GpioAccessor {
 private:
  GpioStm32<PORT> gpio_{};

 public:
  [[nodiscard]] GpioPeripheral& get() noexcept { return gpio_; }

  [[nodiscard]] GpioPeripheral& gpio0() noexcept
    requires(PORT == 0)
  {
    return get();
  }
  [[nodiscard]] GpioPeripheral& gpio_a() noexcept
    requires(PORT == 0)
  {
    return get();
  }

  [[nodiscard]] GpioPeripheral& gpio1() noexcept
    requires(PORT == 1)
  {
    return get();
  }
  [[nodiscard]] GpioPeripheral& gpio_b() noexcept
    requires(PORT == 1)
  {
    return get();
  }

  [[nodiscard]] GpioPeripheral& gpio2() noexcept
    requires(PORT == 2)
  {
    return get();
  }
  [[nodiscard]] GpioPeripheral& gpio_c() noexcept
    requires(PORT == 2)
  {
    return get();
  }

  [[nodiscard]] GpioPeripheral& gpio3() noexcept
    requires(PORT == 3)
  {
    return get();
  }
  [[nodiscard]] GpioPeripheral& gpio_d() noexcept
    requires(PORT == 3)
  {
    return get();
  }

  [[nodiscard]] GpioPeripheral& gpio4() noexcept
    requires(PORT == 4)
  {
    return get();
  }
  [[nodiscard]] GpioPeripheral& gpio_e() noexcept
    requires(PORT == 4)
  {
    return get();
  }

  [[nodiscard]] GpioPeripheral& gpio5() noexcept
    requires(PORT == 5)
  {
    return get();
  }
  [[nodiscard]] GpioPeripheral& gpio_f() noexcept
    requires(PORT == 5)
  {
    return get();
  }

  [[nodiscard]] GpioPeripheral& gpio6() noexcept
    requires(PORT == 6)
  {
    return get();
  }
  [[nodiscard]] GpioPeripheral& gpio_g() noexcept
    requires(PORT == 6)
  {
    return get();
  }

  [[nodiscard]] GpioPeripheral& gpio7() noexcept
    requires(PORT == 7)
  {
    return get();
  }
  [[nodiscard]] GpioPeripheral& gpio_h() noexcept
    requires(PORT == 7)
  {
    return get();
  }
};

}  // namespace tvsc::hal2::gpio
