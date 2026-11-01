#pragma once

#include <cstddef>

#include "hal2/config/facility.h"
#include "hal2/gpio/gpio.h"

namespace tvsc::hal2::led {

template <typename System, typename PeripheralRegistry, size_t INDEX, gpio::PinRef PIN>
class LedAccessor;

template <typename System, typename PeripheralRegistry, gpio::PinRef PIN>
class LedAccessor<System, PeripheralRegistry, 0, PIN> {
 private:
  static constexpr config::PeripheralId gpio_id{config::Facility::GPIO_FACILITY, PIN.port};

  using GpioBankAccessor = config::accessor_type<System, PeripheralRegistry, gpio_id>;

  gpio::PinPeripheral led_{static_cast<GpioBankAccessor&>(static_cast<System&>(*this)).get(),
                           PIN.pin};

 public:
  [[nodiscard]] auto& get() noexcept { return led_; }

  [[nodiscard]] auto& led0() noexcept { return get(); }
  [[nodiscard]] auto& debug_led() noexcept { return get(); }
};

template <typename System, typename PeripheralRegistry, gpio::PinRef PIN>
class LedAccessor<System, PeripheralRegistry, 1, PIN> {
 private:
  static constexpr config::PeripheralId gpio_id{config::Facility::GPIO_FACILITY, PIN.port};

  using GpioBankAccessor = config::accessor_type<System, PeripheralRegistry, gpio_id>;

  gpio::PinPeripheral led_{static_cast<GpioBankAccessor&>(static_cast<System&>(*this)).get(),
                           PIN.pin};

 public:
  [[nodiscard]] auto& get() noexcept { return led_; }

  [[nodiscard]] auto& led1() noexcept { return get(); }
};

template <typename System, typename PeripheralRegistry, gpio::PinRef PIN>
class LedAccessor<System, PeripheralRegistry, 2, PIN> {
 private:
  static constexpr config::PeripheralId gpio_id{config::Facility::GPIO_FACILITY, PIN.port};

  using GpioBankAccessor = config::accessor_type<System, PeripheralRegistry, gpio_id>;

  gpio::PinPeripheral led_{static_cast<GpioBankAccessor&>(static_cast<System&>(*this)).get(),
                           PIN.pin};

 public:
  [[nodiscard]] auto& get() noexcept { return led_; }

  [[nodiscard]] auto& led2() noexcept { return get(); }
};

template <typename System, typename PeripheralRegistry, gpio::PinRef PIN>
class LedAccessor<System, PeripheralRegistry, 3, PIN> {
 private:
  static constexpr config::PeripheralId gpio_id{config::Facility::GPIO_FACILITY, PIN.port};

  using GpioBankAccessor = config::accessor_type<System, PeripheralRegistry, gpio_id>;

  gpio::PinPeripheral led_{static_cast<GpioBankAccessor&>(static_cast<System&>(*this)).get(),
                           PIN.pin};

 public:
  [[nodiscard]] auto& get() noexcept { return led_; }

  [[nodiscard]] auto& led3() noexcept { return get(); }
};

}  // namespace tvsc::hal2::led
