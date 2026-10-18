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
class Stm32GpioAccessor {
 private:
  GpioStm32 gpio_;

  static GpioStm32 initialize_gpio() noexcept {
    if constexpr (PORT == 0) {
#if defined(GPIOA_BASE)
      return GpioStm32{reinterpret_cast<void*>(GPIOA_BASE), PORT};
#endif
    } else if constexpr (PORT == 1) {
#if defined(GPIOB_BASE)
      return GpioStm32{reinterpret_cast<void*>(GPIOB_BASE), PORT};
#endif
    } else if constexpr (PORT == 2) {
#if defined(GPIOC_BASE)
      return GpioStm32{reinterpret_cast<void*>(GPIOC_BASE), PORT};
#endif
    } else if constexpr (PORT == 3) {
#if defined(GPIOD_BASE)
      return GpioStm32{reinterpret_cast<void*>(GPIOD_BASE), PORT};
#endif
    } else if constexpr (PORT == 4) {
#if defined(GPIOE_BASE)
      return GpioStm32{reinterpret_cast<void*>(GPIOE_BASE), PORT};
#endif
    } else if constexpr (PORT == 5) {
#if defined(GPIOF_BASE)
      return GpioStm32{reinterpret_cast<void*>(GPIOF_BASE), PORT};
#endif
    } else if constexpr (PORT == 6) {
#if defined(GPIOG_BASE)
      return GpioStm32{reinterpret_cast<void*>(GPIOG_BASE), PORT};
#endif
    } else if constexpr (PORT == 7) {
#if defined(GPIOH_BASE)
      return GpioStm32{reinterpret_cast<void*>(GPIOH_BASE), PORT};
#endif
    }

    except<std::domain_error>("Unimplemented or nonexistent GPIO port.");
  }

 public:
  Stm32GpioAccessor() noexcept : gpio_(initialize_gpio()) {}

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

template <PortNumber PORT>
struct Stm32GpioConfiguration {
  template <typename System>
  using accessor = Stm32GpioAccessor<PORT, System>;

  [[nodiscard]] static constexpr auto facility() noexcept {
    return config::Facility::GPIO_FACILITY;
  }
  [[nodiscard]] static constexpr size_t index() noexcept { return PORT; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    // No ISRs required.
  }
};
static_assert(config::ValidPeripheralConfiguration<Stm32GpioConfiguration<0>>);

}  // namespace tvsc::hal2::gpio
