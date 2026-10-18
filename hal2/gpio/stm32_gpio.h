#pragma once

#include <cstddef>
#include <cstdint>
#include <new>

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

class GpioStm32 final : public GpioPeripheral {
 private:
  GpioRegisterBank* registers_;
  const PortNumber port_;

  void set_ospeedr_value(PinNumber pin, PinSpeed speed) noexcept;

  enum class MODER_VALUES : uint8_t {
    INPUT = 0b00,
    OUTPUT = 0b01,
    ALTERNATE_FUNCTION = 0b10,
    ANALOG = 0b11,
  };
  void set_moder_value(PinNumber pin, MODER_VALUES value) noexcept;

  enum class OPUPDR_VALUES : uint8_t {
    BOTH_DISABLED = 0x00,
    PULL_UP_ENABLED = 0b01,
    PULL_DOWN_ENABLED = 0b10,
  };
  void set_pupdr_value(PinNumber pin, OPUPDR_VALUES value) noexcept;

  enum class OTYPER_VALUES : uint8_t {
    MOSFET_PUSH_PULL = 0b0,
    MOSFET_OPEN_DRAIN = 0b1,
  };
  void set_otyper_value(PinNumber pin, OTYPER_VALUES value) noexcept;

  void set_alternate_function(PinNumber pin, uint8_t value) noexcept;

  void enable() noexcept override;
  void disable() noexcept override;

  void set_pin_mode(PinNumber pin, PinMode mode, PinSpeed speed,
                    uint8_t alternate_function_mapping) noexcept override;

  void write_pin(PinNumber pin, bool on) noexcept override;
  void toggle_pin(PinNumber pin) noexcept override;

  [[nodiscard]] bool read_pin(PinNumber pin) noexcept override;

 public:
  static constexpr size_t NUM_PINS{16};

  GpioStm32(void* base_address, PortNumber port) noexcept
      : registers_(new(base_address) GpioRegisterBank), port_(port) {}

  [[nodiscard]] PortNumber port() const noexcept override;
};

}  // namespace tvsc::hal2::gpio
