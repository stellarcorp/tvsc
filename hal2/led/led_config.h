#pragma once

#include <cstddef>

#include "hal2/device.h"
#include "hal2/gpio/gpio.h"

namespace tvsc::hal2::led {

template <typename DeviceRegistry, typename Board, size_t INDEX, gpio::PinRef PIN>
class LedAccessor;

template <typename DeviceRegistry, typename Board, gpio::PinRef PIN>
class LedAccessor<DeviceRegistry, Board, 0, PIN> {
 private:
  static constexpr DeviceId gpio_id{Facility::GPIO_FACILITY, PIN.port};

  using GpioBankAccessor = registry::accessor_type<DeviceRegistry, Board, gpio_id>;

  gpio::PinDevice led_{static_cast<GpioBankAccessor&>(static_cast<DeviceRegistry&>(*this)).get(),
                           PIN.pin};

 public:
  [[nodiscard]] auto& get() noexcept { return led_; }

  [[nodiscard]] auto& led0() noexcept { return get(); }
  [[nodiscard]] auto& debug_led() noexcept { return get(); }
};

template <typename DeviceRegistry, typename Board, gpio::PinRef PIN>
class LedAccessor<DeviceRegistry, Board, 1, PIN> {
 private:
  static constexpr DeviceId gpio_id{Facility::GPIO_FACILITY, PIN.port};

  using GpioBankAccessor = registry::accessor_type<DeviceRegistry, Board, gpio_id>;

  gpio::PinDevice led_{static_cast<GpioBankAccessor&>(static_cast<DeviceRegistry&>(*this)).get(),
                           PIN.pin};

 public:
  [[nodiscard]] auto& get() noexcept { return led_; }

  [[nodiscard]] auto& led1() noexcept { return get(); }
};

template <typename DeviceRegistry, typename Board, gpio::PinRef PIN>
class LedAccessor<DeviceRegistry, Board, 2, PIN> {
 private:
  static constexpr DeviceId gpio_id{Facility::GPIO_FACILITY, PIN.port};

  using GpioBankAccessor = registry::accessor_type<DeviceRegistry, Board, gpio_id>;

  gpio::PinDevice led_{static_cast<GpioBankAccessor&>(static_cast<DeviceRegistry&>(*this)).get(),
                           PIN.pin};

 public:
  [[nodiscard]] auto& get() noexcept { return led_; }

  [[nodiscard]] auto& led2() noexcept { return get(); }
};

template <typename DeviceRegistry, typename Board, gpio::PinRef PIN>
class LedAccessor<DeviceRegistry, Board, 3, PIN> {
 private:
  static constexpr DeviceId gpio_id{Facility::GPIO_FACILITY, PIN.port};

  using GpioBankAccessor = registry::accessor_type<DeviceRegistry, Board, gpio_id>;

  gpio::PinDevice led_{static_cast<GpioBankAccessor&>(static_cast<DeviceRegistry&>(*this)).get(),
                           PIN.pin};

 public:
  [[nodiscard]] auto& get() noexcept { return led_; }

  [[nodiscard]] auto& led3() noexcept { return get(); }
};

}  // namespace tvsc::hal2::led
