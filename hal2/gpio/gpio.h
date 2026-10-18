#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include "hal2/peripheral.h"

namespace tvsc::hal2::gpio {

/**
 * Type used to identify a GPIO port (specific peripheral) on an processor.
 */
using PortNumber = uint8_t;

/**
 * Type used to identify a Pin on a GPIO port.
 */
using PinNumber = uint8_t;

/**
 * Reference to a specific pin on a specific GPIO port.
 */
struct PinRef final {
  PortNumber port{};
  PinNumber pin{};
};

/**
 * Union across all relevant platforms of all possible modes for a pin. Some modes will not be
 * supported on some platforms.
 */
enum class PinMode : uint8_t {
  // Pin is unused and should be put in the state that uses the least amount of power. For most
  // pins (typically any pin not involved in debug), this is likely to be the reset state.
  UNUSED,

  // Analog input or output. Pull-up and pull-down resistors and MOSFETs controlled through
  // functions managing analog resolution, voltage reference, etc.
  ANALOG,

  // Digital input with pull-up and pull-down resistors disabled.
  INPUT_FLOATING,

  // Digital input with pull-up resistor enabled and pull-down resistor disabled.
  INPUT_WITH_PULL_UP,

  // Digital input with pull-up resistor disabled and pull-down resistor enabled.
  INPUT_WITH_PULL_DOWN,

  // Digital output with MOSFETs in a push-pull configuration. Pull-up and pull-down resistors
  // disabled.
  OUTPUT_PUSH_PULL,

  // Digital output with MOSFETs in a push-pull configuration. Pull-up resistor enabled. Pull-down
  // resistor disabled.
  OUTPUT_PUSH_PULL_WITH_PULL_UP,

  // Digital output with MOSFETs in a push-pull configuration. Pull-up resistor disabled. Pull-down
  // resistor enabled.
  OUTPUT_PUSH_PULL_WITH_PULL_DOWN,

  // Digital output with MOSFETs in an open drain configuration. Pull-up and pull-down resistors
  // disabled.
  OUTPUT_OPEN_DRAIN,

  // Digital output with MOSFETs in an open drain configuration. Pull-up resistor enabled. Pull-down
  // resistor disabled.
  OUTPUT_OPEN_DRAIN_WITH_PULL_UP,

  // Digital output with MOSFETs in an open drain configuration. Pull-up resistor disabled.
  // Pull-down resistor enabled.
  OUTPUT_OPEN_DRAIN_WITH_PULL_DOWN,

  // Alternate functions. Using these modes allows the pin to be used by another peripheral and not
  // as a general purpose IO pin.

  // Pin set to alternate function with MOSFETs in a push-pull configuration. Pull-up and pull-down
  // resistors disabled.
  ALTERNATE_FUNCTION_PUSH_PULL,

  // Pin set to alternate function with MOSFETs in a push-pull configuration. Pull-up resistor
  // enabled. Pull-down resistor disabled.
  ALTERNATE_FUNCTION_PUSH_PULL_WITH_PULL_UP,

  // Pin set to alternate function with MOSFETs in a push-pull configuration. Pull-up resistor
  // disabled. Pull-down resistor enabled.
  ALTERNATE_FUNCTION_PUSH_PULL_WITH_PULL_DOWN,

  // Pin set to alternate function with MOSFETs in an open drain configuration. Pull-up and
  // pull-down resistors disabled.
  ALTERNATE_FUNCTION_OPEN_DRAIN,

  // Pin set to alternate function with MOSFETs in an open drain configuration. Pull-up resistor
  // enabled. Pull-down resistor disabled.
  ALTERNATE_FUNCTION_OPEN_DRAIN_WITH_PULL_UP,

  // Pin set to alternate function with MOSFETs in an open drain configuration. Pull-up resistor
  // disabled. Pull-down resistor enabled.
  ALTERNATE_FUNCTION_OPEN_DRAIN_WITH_PULL_DOWN,
};

enum class PinSpeed : uint8_t {
  LOW,
  MEDIUM,
  HIGH,
  VERY_HIGH,
};

class Gpio;

class GpioPeripheral : public Peripheral<GpioPeripheral, Gpio> {
 private:
  virtual void enable() noexcept = 0;
  virtual void disable() noexcept = 0;

  virtual void set_pin_mode(PinNumber pin, PinMode mode, PinSpeed speed,
                            uint8_t alternate_function_mapping) noexcept = 0;

  [[nodiscard]] virtual bool read_pin(PinNumber pin) noexcept = 0;
  virtual void write_pin(PinNumber pin, bool on) noexcept = 0;
  virtual void toggle_pin(PinNumber pin) noexcept = 0;

  friend class Gpio;

  template <typename ClockType>
  friend class GpioInterceptor;

 public:
  virtual ~GpioPeripheral() = default;

  [[nodiscard]] virtual PortNumber port() const noexcept = 0;
};

class Gpio final : public Driver<GpioPeripheral, Gpio> {
 protected:
  explicit Gpio(GpioPeripheral& peripheral) : Driver<GpioPeripheral, Gpio>(peripheral) {}

  friend class Peripheral<GpioPeripheral, Gpio>;

 public:
  constexpr Gpio() noexcept = default;

  void set_pin_mode(PinNumber pin, PinMode mode, PinSpeed speed = PinSpeed::LOW,
                    uint8_t alternate_function_mapping = 0) noexcept {
    peripheral_->set_pin_mode(pin, mode, speed, alternate_function_mapping);
  }

  [[nodiscard]] bool read_pin(PinNumber pin) noexcept { return peripheral_->read_pin(pin); }
  void write_pin(PinNumber pin, bool on) noexcept { peripheral_->write_pin(pin, on); }
  void toggle_pin(PinNumber pin) noexcept { peripheral_->toggle_pin(pin); }

  [[nodiscard]] PortNumber port() const noexcept { return peripheral_->port(); }
};

class Pin;

class PinPeripheral final : public Peripheral<PinPeripheral, Pin> {
 private:
  GpioPeripheral* gpio_peripheral_;
  const PinNumber pin_number_;
  Gpio gpio_{};

  void enable() noexcept override { gpio_.ensure_valid(*gpio_peripheral_); }

  void disable() noexcept override { gpio_.invalidate(); }

  void set_pin_mode(PinMode mode, PinSpeed speed, uint8_t alternate_function_mapping) noexcept {
    gpio_.set_pin_mode(pin_number_, mode, speed, alternate_function_mapping);
  }

  [[nodiscard]] bool read_pin() noexcept { return gpio_.read_pin(pin_number_); }
  void write_pin(bool on) noexcept { gpio_.write_pin(pin_number_, on); }
  void toggle_pin() noexcept { gpio_.toggle_pin(pin_number_); }

  friend class Pin;

 public:
  constexpr PinPeripheral(GpioPeripheral& gpio_peripheral, PinNumber pin_number) noexcept
      : gpio_peripheral_(&gpio_peripheral), pin_number_(pin_number) {}

  // constexpr PinPeripheral(const PinPeripheral&) noexcept = default;
  constexpr PinPeripheral(PinPeripheral&&) noexcept = default;
  // constexpr PinPeripheral& operator=(const PinPeripheral&) noexcept = default;
  constexpr PinPeripheral& operator=(PinPeripheral&&) noexcept = default;

  [[nodiscard]] constexpr PortNumber port() const noexcept { return gpio_peripheral_->port(); }
  [[nodiscard]] constexpr PinNumber pin() const noexcept { return pin_number_; }
  [[nodiscard]] constexpr PinRef ref() const noexcept {
    return {gpio_peripheral_->port(), pin_number_};
  }
};

class Pin final : public Driver<PinPeripheral, Pin> {
 private:
  friend class Peripheral<PinPeripheral, Pin>;

  explicit Pin(PinPeripheral& peripheral) noexcept : Driver<PinPeripheral, Pin>(peripheral) {}

 public:
  Pin() noexcept = default;

  void set_pin_mode(PinMode mode, PinSpeed speed = PinSpeed::LOW,
                    uint8_t alternate_function_mapping = 0) noexcept {
    peripheral_->set_pin_mode(mode, speed, alternate_function_mapping);
  }

  [[nodiscard]] bool read_pin() noexcept { return peripheral_->read_pin(); }
  void write_pin(bool on) noexcept { peripheral_->write_pin(on); }
  void on() noexcept { write_pin(true); }
  void off() noexcept { write_pin(false); }
  void toggle_pin() noexcept { peripheral_->toggle_pin(); }

  [[nodiscard]] PortNumber port() const noexcept { return peripheral_->port(); }
  [[nodiscard]] PinNumber pin() const noexcept { return peripheral_->pin(); }
  [[nodiscard]] PinRef ref() const noexcept { return peripheral_->ref(); }
};

}  // namespace tvsc::hal2::gpio
