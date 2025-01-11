#pragma once

//#include "base/except.h"
#include <new>

#include "hal/boards/register.h"
#include "hal/gpio/gpio.h"
#include "hal/gpio/pins.h"

extern "C" {
#include "stm32h7xx.h"
}

namespace tvsc::hal::boards::nucleo_h743zi {

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

class GpioStm32H7xx final : public gpio::Gpio {
 private:
  GpioRegisterBank* registers_;

  void set_ospeedr_value(gpio::Pin pin, gpio::PinSpeed speed) {
    uint8_t speed_value{};
    switch (speed) {
      case gpio::PinSpeed::LOW:
        speed_value = 0b00;
        break;
      case gpio::PinSpeed::MEDIUM:
        speed_value = 0b01;
        break;
      case gpio::PinSpeed::HIGH:
        speed_value = 0b10;
        break;
      case gpio::PinSpeed::VERY_HIGH:
        speed_value = 0b11;
        break;
        // default:
        // except<std::domain_error>("Requested pin speed is not supported on this platform.");
    }
    registers_->OSPEEDR.set_bit_field_value<2>(speed_value, static_cast<uint8_t>(2 * pin));
  }

  enum class MODER_VALUES : uint8_t {
    INPUT = 0b00,
    OUTPUT = 0b01,
    ALTERNATE_FUNCTION = 0b10,
    ANALOG = 0b11,
  };
  void set_moder_value(gpio::Pin pin, MODER_VALUES value) {
    registers_->MODER.set_bit_field_value<2>(static_cast<uint32_t>(value),
                                             static_cast<uint8_t>(2 * pin));
  }

  enum class OPUPDR_VALUES : uint8_t {
    BOTH_DISABLED = 0x00,
    PULL_UP_ENABLED = 0b01,
    PULL_DOWN_ENABLED = 0b10,
  };
  void set_pupdr_value(gpio::Pin pin, OPUPDR_VALUES value) {
    registers_->OPUPDR.set_bit_field_value<2>(static_cast<uint32_t>(value),
                                              static_cast<uint8_t>(2 * pin));
  }

  enum class OTYPER_VALUES : uint8_t {
    MOSFET_PUSH_PULL = 0b0,
    MOSFET_OPEN_DRAIN = 0b1,
  };
  void set_otyper_value(gpio::Pin pin, OTYPER_VALUES value) {
    registers_->OTYPER.set_bit_field_value<1>(static_cast<uint32_t>(value),
                                              static_cast<uint8_t>(pin));
  }

 public:
  static constexpr size_t NUM_PINS{16};

  GpioStm32H7xx(void* base_address) : registers_(new (base_address) GpioRegisterBank) {
    // TODO(james): Move pin configuration out of this constructor. This class should provide an API
    // that allows for this configuration, but it should not dictate the configuration of any pin.
  }

  void set_pin_mode(gpio::Pin pin, gpio::PinMode mode, gpio::PinSpeed speed) override {
    switch (mode) {
        // Unused pins get set to the reset state of most pins. Note that the reset state of some
        // pins, especially debug pins, is different.
      case gpio::PinMode::UNUSED:
        set_moder_value(pin, MODER_VALUES::ANALOG);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
        break;

        // Analog mode needs the mode configured, and the pull-up/pull-down resistors must be
        // disabled. Speed and type are ignored.
      case gpio::PinMode::ANALOG:
        set_moder_value(pin, MODER_VALUES::ANALOG);
        set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
        break;

        // Input modes need the mode and pull-up/pull-down configured. Speed and type are ignored.
      case gpio::PinMode::INPUT_FLOATING:
        set_moder_value(pin, MODER_VALUES::INPUT);
        set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
        break;

      case gpio::PinMode::INPUT_WITH_PULL_UP:
        set_moder_value(pin, MODER_VALUES::INPUT);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_UP_ENABLED);
        break;

      case gpio::PinMode::INPUT_WITH_PULL_DOWN:
        set_moder_value(pin, MODER_VALUES::INPUT);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_DOWN_ENABLED);
        break;

        // Output modes need the mode, output type, output speed, and pull-up/pull-down configured.
      case gpio::PinMode::OUTPUT_PUSH_PULL:
        set_moder_value(pin, MODER_VALUES::OUTPUT);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
        break;

      case gpio::PinMode::OUTPUT_PUSH_PULL_WITH_PULL_UP:
        set_moder_value(pin, MODER_VALUES::OUTPUT);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_UP_ENABLED);
        break;

      case gpio::PinMode::OUTPUT_PUSH_PULL_WITH_PULL_DOWN:
        set_moder_value(pin, MODER_VALUES::OUTPUT);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_DOWN_ENABLED);
        break;

      case gpio::PinMode::OUTPUT_OPEN_DRAIN:
        set_moder_value(pin, MODER_VALUES::OUTPUT);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
        break;

      case gpio::PinMode::OUTPUT_OPEN_DRAIN_WITH_PULL_UP:
        set_moder_value(pin, MODER_VALUES::OUTPUT);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_UP_ENABLED);
        break;

      case gpio::PinMode::OUTPUT_OPEN_DRAIN_WITH_PULL_DOWN:
        set_moder_value(pin, MODER_VALUES::OUTPUT);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_DOWN_ENABLED);
        break;

        // Alternate function modes need the mode, type, speed, and pull-up/pull-down configured.
      case gpio::PinMode::ALTERNATE_FUNCTION_PUSH_PULL:
        set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
        break;

      case gpio::PinMode::ALTERNATE_FUNCTION_PUSH_PULL_WITH_PULL_UP:
        set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_UP_ENABLED);
        break;

      case gpio::PinMode::ALTERNATE_FUNCTION_PUSH_PULL_WITH_PULL_DOWN:
        set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_DOWN_ENABLED);
        break;

      case gpio::PinMode::ALTERNATE_FUNCTION_OPEN_DRAIN:
        set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
        break;

      case gpio::PinMode::ALTERNATE_FUNCTION_OPEN_DRAIN_WITH_PULL_UP:
        set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_UP_ENABLED);
        break;

      case gpio::PinMode::ALTERNATE_FUNCTION_OPEN_DRAIN_WITH_PULL_DOWN:
        set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
        set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
        set_ospeedr_value(pin, speed);
        set_pupdr_value(pin, OPUPDR_VALUES::PULL_DOWN_ENABLED);
        break;

        // default:
        // except<std::domain_error>("Requested pin mode is not supported on this platform.");
    }
  }

  void write_pin(gpio::Pin pin, bool on) override {
    // Atomically turn on or off the pin. Setting a one in the lower 16-bits turns ON the pin.
    // Setting a one in the upper 16-bits turns OFF the pin.
    if (on) {
      registers_->BSRR.set_value(pin);
    } else {
      registers_->BSRR.set_value(pin << NUM_PINS);
    }
  }
  void toggle_pin(gpio::Pin pin) override {
    const uint32_t current_value{registers_->ODR.value()};
    registers_->BSRR.set_value(((current_value & (1U << pin)) << NUM_PINS) |
                               (~current_value & (1U << pin)));
  }
};

}  // namespace tvsc::hal::boards::nucleo_h743zi
