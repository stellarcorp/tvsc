#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include "hal/peripheral.h"

namespace tvsc::hal::gpio {

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
  virtual void enable() = 0;
  virtual void disable() = 0;

  virtual void set_pin_mode(PinNumber pin, PinMode mode, PinSpeed speed,
                            uint8_t alternate_function_mapping) = 0;

  virtual bool read_pin(PinNumber pin) = 0;
  virtual void write_pin(PinNumber pin, bool on) = 0;
  virtual void toggle_pin(PinNumber pin) = 0;

  friend class Gpio;

  template <typename ClockType>
  friend class GpioInterceptor;

 public:
  virtual ~GpioPeripheral() = default;

  virtual PortNumber port() const = 0;
};

class Gpio final : public Functional<GpioPeripheral, Gpio> {
 protected:
  explicit Gpio(GpioPeripheral& peripheral) : Functional<GpioPeripheral, Gpio>(peripheral) {}

  friend class Peripheral<GpioPeripheral, Gpio>;

 public:
  Gpio() = default;

  void set_pin_mode(PinNumber pin, PinMode mode, PinSpeed speed = PinSpeed::LOW,
                    uint8_t alternate_function_mapping = 0) {
    peripheral_->set_pin_mode(pin, mode, speed, alternate_function_mapping);
  }

  bool read_pin(PinNumber pin) { return peripheral_->read_pin(pin); }
  void write_pin(PinNumber pin, bool on) { peripheral_->write_pin(pin, on); }
  void toggle_pin(PinNumber pin) { peripheral_->toggle_pin(pin); }

  PortNumber port() const { return peripheral_->port(); }
};

class Pin;

class PinPeripheral final : public Peripheral<PinPeripheral, Pin> {
 private:
  GpioPeripheral* gpio_peripheral_;
  const PinNumber pin_number_;
  Gpio gpio_{};

  void enable() override { gpio_.ensure_valid(*gpio_peripheral_); }

  void disable() override { gpio_.invalidate(); }

  void set_pin_mode(PinMode mode, PinSpeed speed, uint8_t alternate_function_mapping) {
    gpio_.set_pin_mode(pin_number_, mode, speed, alternate_function_mapping);
  }

  bool read_pin() { return gpio_.read_pin(pin_number_); }
  void write_pin(bool on) { gpio_.write_pin(pin_number_, on); }
  void toggle_pin() { gpio_.toggle_pin(pin_number_); }

  friend class Pin;

 public:
  PinPeripheral(GpioPeripheral& gpio_peripheral, PinNumber pin_number)
      : gpio_peripheral_(&gpio_peripheral), pin_number_(pin_number) {}

  PortNumber port() const { return gpio_peripheral_->port(); }
  PinNumber pin() const { return pin_number_; }
};

class Pin final : public Functional<PinPeripheral, Pin> {
 private:
  friend class Peripheral<PinPeripheral, Pin>;

  explicit Pin(PinPeripheral& peripheral) : Functional<PinPeripheral, Pin>(peripheral) {}

 public:
  void set_pin_mode(PinMode mode, PinSpeed speed = PinSpeed::LOW,
                    uint8_t alternate_function_mapping = 0) {
    peripheral_->set_pin_mode(mode, speed, alternate_function_mapping);
  }

  bool read_pin() { return peripheral_->read_pin(); }
  void write_pin(bool on) { peripheral_->write_pin(on); }
  void toggle_pin() { peripheral_->toggle_pin(); }

  PortNumber port() const { return peripheral_->port(); }
  PinNumber pin() const { return peripheral_->pin(); }
};

}  // namespace tvsc::hal::gpio
