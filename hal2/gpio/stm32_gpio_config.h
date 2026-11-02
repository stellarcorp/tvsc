#pragma once

#include <stdexcept>

#include "base/except.h"
#include "hal2/device.h"
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
  Stm32Gpio<0> gpio_{};

 public:
  [[nodiscard]] GpioDevice& get() noexcept { return gpio_; }

  [[nodiscard]] GpioDevice& gpio0() noexcept { return get(); }

  [[nodiscard]] GpioDevice& gpio_a() noexcept { return get(); }
};

template <typename System>
class Stm32GpioAccessor<1, System> {
 private:
  Stm32Gpio<1> gpio_{};

 public:
  [[nodiscard]] GpioDevice& get() noexcept { return gpio_; }

  [[nodiscard]] GpioDevice& gpio1() noexcept { return get(); }

  [[nodiscard]] GpioDevice& gpio_b() noexcept { return get(); }
};

template <typename System>
class Stm32GpioAccessor<2, System> {
 private:
  Stm32Gpio<2> gpio_{};

 public:
  [[nodiscard]] GpioDevice& get() noexcept { return gpio_; }

  [[nodiscard]] GpioDevice& gpio2() noexcept { return get(); }

  [[nodiscard]] GpioDevice& gpio_c() noexcept { return get(); }
};

template <typename System>
class Stm32GpioAccessor<3, System> {
 private:
  Stm32Gpio<3> gpio_{};

 public:
  [[nodiscard]] GpioDevice& get() noexcept { return gpio_; }

  [[nodiscard]] GpioDevice& gpio3() noexcept { return get(); }

  [[nodiscard]] GpioDevice& gpio_d() noexcept { return get(); }
};

template <typename System>
class Stm32GpioAccessor<4, System> {
 private:
  Stm32Gpio<4> gpio_{};

 public:
  [[nodiscard]] GpioDevice& get() noexcept { return gpio_; }

  [[nodiscard]] GpioDevice& gpio4() noexcept { return get(); }

  [[nodiscard]] GpioDevice& gpio_e() noexcept { return get(); }
};

template <typename System>
class Stm32GpioAccessor<5, System> {
 private:
  Stm32Gpio<5> gpio_{};

 public:
  [[nodiscard]] GpioDevice& get() noexcept { return gpio_; }

  [[nodiscard]] GpioDevice& gpio5() noexcept { return get(); }

  [[nodiscard]] GpioDevice& gpio_f() noexcept { return get(); }
};

template <typename System>
class Stm32GpioAccessor<6, System> {
 private:
  Stm32Gpio<6> gpio_{};

 public:
  [[nodiscard]] GpioDevice& get() noexcept { return gpio_; }

  [[nodiscard]] GpioDevice& gpio6() noexcept { return get(); }

  [[nodiscard]] GpioDevice& gpio_g() noexcept { return get(); }
};

template <typename System>
class Stm32GpioAccessor<7, System> {
 private:
  Stm32Gpio<7> gpio_{};

 public:
  [[nodiscard]] GpioDevice& get() noexcept { return gpio_; }

  [[nodiscard]] GpioDevice& gpio7() noexcept { return get(); }

  [[nodiscard]] GpioDevice& gpio_h() noexcept { return get(); }
};

}  // namespace tvsc::hal2::gpio
