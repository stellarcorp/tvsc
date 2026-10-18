#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <stdexcept>

#include "base/except.h"
#include "hal2/gpio/gpio.h"
#include "hal2/register.h"

namespace tvsc::hal2::gpio {

class GpioRegisterBank final {
 public:
  // Configures the mode of a pin. One of input, output, alternate function, or analog.
  // Offset 0x00
  volatile Register MODER;

  // Configures the output type of a pin.
  // Offset 0x04
  volatile Register OTYPER;

  // Configures the output speed of a pin.
  // Offset 0x08
  volatile Register OSPEEDR;

  // Configures pull-up or pull-down of a pin.
  // Offset 0x0c.
  volatile Register OPUPDR;

  // Input data register for reading the current digital value of the pins in a GPIO port.
  // Offset 0x10
  volatile Register IDR;

  // Output data register for reading or writing the digital value being output on the pins in a
  // GPIO port.
  // Offset 0x14
  volatile Register ODR;

  // Sets or resets pins atomically. This register always reads zero for all pins; its purpose is to
  // set the value of the pin atomically regardless of the pin's current value. It accomplishes this
  // atomicity by being write-only.
  // Offset 0x18
  volatile Register BSRR;

  // Locks the configuration of the pins in this port.
  // Offset 0x1c
  volatile Register LCKR;

  // Configures the alternate function for pins 0-7, 4 bits per pin.
  // Offset 0x20
  volatile Register AFRL;
  // Configures the alternate function for pins 8-15, 4 bits per pin.
  // Offset 0x24
  volatile Register AFRH;
};

template <PortNumber PORT>
class GpioStm32 final : public GpioPeripheral {
 private:
  GpioRegisterBank* registers_{instantiate_registers()};

  static GpioRegisterBank* instantiate_registers() noexcept {
    if constexpr (PORT == 0) {
#if defined(GPIOA_BASE)
      return new (GPIOA_BASE) GpioRegisterBank;
#endif
    } else if constexpr (PORT == 1) {
#if defined(GPIOB_BASE)
      return new (GPIOB_BASE) GpioRegisterBank;
#endif
    } else if constexpr (PORT == 2) {
#if defined(GPIOC_BASE)
      return new (GPIOC_BASE) GpioRegisterBank;
#endif
    } else if constexpr (PORT == 3) {
#if defined(GPIOD_BASE)
      return new (GPIOD_BASE) GpioRegisterBank;
#endif
    } else if constexpr (PORT == 4) {
#if defined(GPIOE_BASE)
      return new (GPIOE_BASE) GpioRegisterBank;
#endif
    } else if constexpr (PORT == 5) {
#if defined(GPIOF_BASE)
      return new (GPIOF_BASE) GpioRegisterBank;
#endif
    } else if constexpr (PORT == 6) {
#if defined(GPIOG_BASE)
      return new (GPIOG_BASE) GpioRegisterBank;
#endif
    } else if constexpr (PORT == 7) {
#if defined(GPIOG_BASE)
      return new (GPIOH_BASE) GpioRegisterBank;
#endif
    }
    except<std::domain_error>("Unimplemented or nonexistent GPIO port.");
  }

  enum class MODER_VALUES : uint8_t {
    INPUT = 0b00,
    OUTPUT = 0b01,
    ALTERNATE_FUNCTION = 0b10,
    ANALOG = 0b11,
  };

  enum class OPUPDR_VALUES : uint8_t {
    BOTH_DISABLED = 0x00,
    PULL_UP_ENABLED = 0b01,
    PULL_DOWN_ENABLED = 0b10,
  };

  enum class OTYPER_VALUES : uint8_t {
    MOSFET_PUSH_PULL = 0b0,
    MOSFET_OPEN_DRAIN = 0b1,
  };

  void set_ospeedr_value(PinNumber pin, PinSpeed speed) noexcept {
    uint8_t speed_value{};
    switch (speed) {
      case PinSpeed::LOW:
        speed_value = 0b00;
        break;
      case PinSpeed::MEDIUM:
        speed_value = 0b01;
        break;
      case PinSpeed::HIGH:
        speed_value = 0b10;
        break;
      case PinSpeed::VERY_HIGH:
        speed_value = 0b11;
        break;
      default:
        except<std::domain_error>("Requested pin speed is not supported on this platform.");
    }
    registers_->OSPEEDR.set_bit_field_value<2>(speed_value, static_cast<uint8_t>(2 * pin));
  }

  void set_moder_value(PinNumber pin, MODER_VALUES value) noexcept {
    registers_->MODER.set_bit_field_value<2>(static_cast<uint32_t>(value),
                                             static_cast<uint8_t>(2 * pin));
  }

  void set_pupdr_value(PinNumber pin, OPUPDR_VALUES value) noexcept {
    registers_->OPUPDR.set_bit_field_value<2>(static_cast<uint32_t>(value),
                                              static_cast<uint8_t>(2 * pin));
  }

  void set_otyper_value(PinNumber pin, OTYPER_VALUES value) noexcept {
    registers_->OTYPER.set_bit_field_value<1>(static_cast<uint32_t>(value),
                                              static_cast<uint8_t>(pin));
  }

  void set_alternate_function(PinNumber pin, uint8_t value) noexcept {
    if (pin >= 0 and pin < 8) {
      registers_->AFRL.set_bit_field_value<4>(value, 4 * pin);
    } else {
      registers_->AFRH.set_bit_field_value<4>(value, 4 * (pin - 8));
    }
  }

  void set_pin_mode(PinNumber pin, PinMode mode, PinSpeed speed,
                    uint8_t alternate_function_mapping) noexcept {
    switch (mode) {
        // Unused pins get set to the reset state of most pins. Note that the reset state of some
        // pins, especially debug pins, is different.
      case PinMode::UNUSED:
        set_moder_value(pin, MODER_VALUES::ANALOG);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
        break;

        // Analog mode needs the mode configured, and the pull-up/pull-down resistors must be
        // disabled. Speed and type are ignored.
      case PinMode::ANALOG:
        set_moder_value(pin, MODER_VALUES::ANALOG);
        set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
        break;

        // Input modes need the mode and pull-up/pull-down configured. Speed and type are ignored.
      case PinMode::INPUT_FLOATING:
        set_moder_value(pin, MODER_VALUES::INPUT);
        set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
        break;

      case PinMode::INPUT_WITH_PULL_UP:
        set_moder_value(pin, MODER_VALUES::INPUT);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_UP_ENABLED);
        break;

      case PinMode::INPUT_WITH_PULL_DOWN:
        set_moder_value(pin, MODER_VALUES::INPUT);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_DOWN_ENABLED);
        break;

        // Output modes need the mode, output type, output speed, and pull-up/pull-down configured.
      case PinMode::OUTPUT_PUSH_PULL:
        set_moder_value(pin, MODER_VALUES::OUTPUT);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
        break;

      case PinMode::OUTPUT_PUSH_PULL_WITH_PULL_UP:
        set_moder_value(pin, MODER_VALUES::OUTPUT);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_UP_ENABLED);
        break;

      case PinMode::OUTPUT_PUSH_PULL_WITH_PULL_DOWN:
        set_moder_value(pin, MODER_VALUES::OUTPUT);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_DOWN_ENABLED);
        break;

      case PinMode::OUTPUT_OPEN_DRAIN:
        set_moder_value(pin, MODER_VALUES::OUTPUT);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
        break;

      case PinMode::OUTPUT_OPEN_DRAIN_WITH_PULL_UP:
        set_moder_value(pin, MODER_VALUES::OUTPUT);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_UP_ENABLED);
        break;

      case PinMode::OUTPUT_OPEN_DRAIN_WITH_PULL_DOWN:
        set_moder_value(pin, MODER_VALUES::OUTPUT);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_DOWN_ENABLED);
        break;

        // Alternate function modes need the mode, type, speed, and pull-up/pull-down configured.
      case PinMode::ALTERNATE_FUNCTION_PUSH_PULL:
        set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
        set_alternate_function(pin, alternate_function_mapping);
        break;

      case PinMode::ALTERNATE_FUNCTION_PUSH_PULL_WITH_PULL_UP:
        set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_UP_ENABLED);
        set_alternate_function(pin, alternate_function_mapping);
        break;

      case PinMode::ALTERNATE_FUNCTION_PUSH_PULL_WITH_PULL_DOWN:
        set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_DOWN_ENABLED);
        set_alternate_function(pin, alternate_function_mapping);
        break;

      case PinMode::ALTERNATE_FUNCTION_OPEN_DRAIN:
        set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
        set_alternate_function(pin, alternate_function_mapping);
        break;

      case PinMode::ALTERNATE_FUNCTION_OPEN_DRAIN_WITH_PULL_UP:
        set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_UP_ENABLED);
        set_alternate_function(pin, alternate_function_mapping);
        break;

      case PinMode::ALTERNATE_FUNCTION_OPEN_DRAIN_WITH_PULL_DOWN:
        set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_DOWN_ENABLED);
        set_alternate_function(pin, alternate_function_mapping);
        break;

      default:
        except<std::domain_error>("Requested pin mode is not supported on this platform.");
    }
  }

  void write_pin(PinNumber pin, bool on) noexcept {
    // Atomically turn on or off the pin. Setting a one in the lower 16-bits turns ON the pin.
    // Setting a one in the upper 16-bits turns OFF the pin.
    if (on) {
      registers_->BSRR.set_value(1U << pin);
    } else {
      registers_->BSRR.set_value((1U << pin) << NUM_PINS);
    }
  }

  void toggle_pin(PinNumber pin) noexcept {
    const uint32_t current_value{registers_->ODR.value()};
    registers_->BSRR.set_value(((current_value & (1U << pin)) << NUM_PINS) |
                               (~current_value & (1U << pin)));
  }

  [[nodiscard]] bool read_pin(PinNumber pin) noexcept {
    return registers_->IDR.bit_field_value<1>(pin);
  }

  void disable() noexcept {
    if constexpr (PORT == 0) {
#if defined(__HAL_RCC_GPIOA_CLK_DISABLE)
      __HAL_RCC_GPIOA_CLK_DISABLE();
#endif
    } else if constexpr (PORT == 1) {
#if defined(__HAL_RCC_GPIOB_CLK_DISABLE)
      __HAL_RCC_GPIOB_CLK_DISABLE();
#endif
    } else if constexpr (PORT == 2) {
#if defined(__HAL_RCC_GPIOC_CLK_DISABLE)
      __HAL_RCC_GPIOC_CLK_DISABLE();
#endif
    } else if constexpr (PORT == 3) {
#if defined(__HAL_RCC_GPIOD_CLK_DISABLE)
      __HAL_RCC_GPIOD_CLK_DISABLE();
#endif
    } else if constexpr (PORT == 4) {
#if defined(__HAL_RCC_GPIOE_CLK_DISABLE)
      __HAL_RCC_GPIOE_CLK_DISABLE();
#endif
    } else if constexpr (PORT == 5) {
#if defined(__HAL_RCC_GPIOF_CLK_DISABLE)
      __HAL_RCC_GPIOF_CLK_DISABLE();
#endif
    } else if constexpr (PORT == 6) {
#if defined(__HAL_RCC_GPIOG_CLK_DISABLE)
      __HAL_RCC_GPIOG_CLK_DISABLE();
#endif
    } else if constexpr (PORT == 7) {
#if defined(__HAL_RCC_GPIOH_CLK_DISABLE)
      __HAL_RCC_GPIOH_CLK_DISABLE();
#endif
    }
  }

  void enable() noexcept {
    if constexpr (PORT == 0) {
#if defined(__HAL_RCC_GPIOA_CLK_ENABLE)
      __HAL_RCC_GPIOA_CLK_ENABLE();
#endif
    } else if constexpr (PORT == 1) {
#if defined(__HAL_RCC_GPIOB_CLK_ENABLE)
      __HAL_RCC_GPIOB_CLK_ENABLE();
#endif
    } else if constexpr (PORT == 2) {
#if defined(__HAL_RCC_GPIOC_CLK_ENABLE)
      __HAL_RCC_GPIOC_CLK_ENABLE();
#endif
    } else if constexpr (PORT == 3) {
#if defined(__HAL_RCC_GPIOD_CLK_ENABLE)
      __HAL_RCC_GPIOD_CLK_ENABLE();
#endif
    } else if constexpr (PORT == 4) {
#if defined(__HAL_RCC_GPIOE_CLK_ENABLE)
      __HAL_RCC_GPIOE_CLK_ENABLE();
#endif
    } else if constexpr (PORT == 5) {
#if defined(__HAL_RCC_GPIOF_CLK_ENABLE)
      __HAL_RCC_GPIOF_CLK_ENABLE();
#endif
    } else if constexpr (PORT == 6) {
#if defined(__HAL_RCC_GPIOG_CLK_ENABLE)
      __HAL_RCC_GPIOG_CLK_ENABLE();
#endif
    } else if constexpr (PORT == 7) {
#if defined(__HAL_RCC_GPIOH_CLK_ENABLE)
      __HAL_RCC_GPIOH_CLK_ENABLE();
#endif
    }
  }

 public:
  static constexpr size_t NUM_PINS{16};

  [[nodiscard]] PortNumber port() const noexcept override { return PORT; }
};

}  // namespace tvsc::hal2::gpio
