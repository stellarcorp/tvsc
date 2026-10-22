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
class Stm32GpioAccessor;

template <typename System>
class Stm32GpioAccessor<0, System> {
 private:
  GpioStm32<0> gpio_{};

 public:
  [[nodiscard]] GpioPeripheral& get() noexcept { return gpio_; }

  [[nodiscard]] GpioPeripheral& gpio0() noexcept { return get(); }

  [[nodiscard]] GpioPeripheral& gpio_a() noexcept { return get(); }
};

template <typename System>
class Stm32GpioAccessor<1, System> {
 private:
  GpioStm32<1> gpio_{};

 public:
  [[nodiscard]] GpioPeripheral& get() noexcept { return gpio_; }

  [[nodiscard]] GpioPeripheral& gpio1() noexcept { return get(); }

  [[nodiscard]] GpioPeripheral& gpio_b() noexcept { return get(); }
};

template <typename System>
class Stm32GpioAccessor<2, System> {
 private:
  GpioStm32<2> gpio_{};

 public:
  [[nodiscard]] GpioPeripheral& get() noexcept { return gpio_; }

  [[nodiscard]] GpioPeripheral& gpio2() noexcept { return get(); }

  [[nodiscard]] GpioPeripheral& gpio_c() noexcept { return get(); }
};

template <typename System>
class Stm32GpioAccessor<3, System> {
 private:
  GpioStm32<3> gpio_{};

 public:
  [[nodiscard]] GpioPeripheral& get() noexcept { return gpio_; }

  [[nodiscard]] GpioPeripheral& gpio3() noexcept { return get(); }

  [[nodiscard]] GpioPeripheral& gpio_d() noexcept { return get(); }
};

template <typename System>
class Stm32GpioAccessor<4, System> {
 private:
  GpioStm32<4> gpio_{};

 public:
  [[nodiscard]] GpioPeripheral& get() noexcept { return gpio_; }

  [[nodiscard]] GpioPeripheral& gpio4() noexcept { return get(); }

  [[nodiscard]] GpioPeripheral& gpio_e() noexcept { return get(); }
};

template <typename System>
class Stm32GpioAccessor<5, System> {
 private:
  GpioStm32<5> gpio_{};

 public:
  [[nodiscard]] GpioPeripheral& get() noexcept { return gpio_; }

  [[nodiscard]] GpioPeripheral& gpio5() noexcept { return get(); }

  [[nodiscard]] GpioPeripheral& gpio_f() noexcept { return get(); }
};

template <typename System>
class Stm32GpioAccessor<6, System> {
 private:
  GpioStm32<6> gpio_{};

 public:
  [[nodiscard]] GpioPeripheral& get() noexcept { return gpio_; }

  [[nodiscard]] GpioPeripheral& gpio6() noexcept { return get(); }

  [[nodiscard]] GpioPeripheral& gpio_g() noexcept { return get(); }
};

template <typename System>
class Stm32GpioAccessor<7, System> {
 private:
  GpioStm32<7> gpio_{};

 public:
  [[nodiscard]] GpioPeripheral& get() noexcept { return gpio_; }

  [[nodiscard]] GpioPeripheral& gpio7() noexcept { return get(); }

  [[nodiscard]] GpioPeripheral& gpio_h() noexcept { return get(); }
};

}  // namespace tvsc::hal2::gpio
