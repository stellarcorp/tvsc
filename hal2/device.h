/**
 * Base classes for peripherals and their functions.
 *
 * The goal is to use RAII to manage power and clock to peripherals; if we aren't using the
 * peripheral's functions, it should be off. These mechanisms are important, since almost all of our
 * use cases will have severe power constraints, and turning off the power and clock whenever
 * possible should extend the lifetime of the satellite. To make this happen, we separate the
 * representation of the Device itself from its function. Every Device has a method to give
 * access to its functions. The base class for the functions, called Driver, increments a
 * reference count on instantiation. The Device then watches the reference count to determine
 * when to enable clock and power to the peripheral and when to disable them.
 *
 * The overall structure hopefully makes it easy to implement new peripherals via subclassing. The
 * Device base class represents a real peripheral on a board, especially one that can be enabled
 * or disabled, but without any access to the functionality of that peripheral. The Driver base
 * class represents access to the functions of the peripheral. The Driver provides an RAII
 * mechanism to control when the peripheral gets power and clock. To create a new peripheral, the
 * typical method will be to subclass both Device and Driver, with the Device acting as
 * the factory for the Driver subclass. The functionality of the peripheral should be exposed
 * only on the Driver subclass. The functionality may be implemented directly on the Driver
 * subclass, or it may delegate back to the Device.
 *
 * In most of our peripherals, we will want to provide another layer between the interface of the
 * functionality and its implementation. This extra layer will allow us to simulate and test the
 * behavior of the satellite by providing fake implementations of the peripherals that work on
 * standard desktop- and server-class computers. These fake implementations will allow us to inject
 * faked sensor readings and validate controls.
 *
 * TODO(james): Device subclasses should be able to contain references to other Devices and
 * the access() method should also guarantee that the dependency's power and clock are managed
 * correctly, likely via the same reference counting mechanism.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

namespace tvsc::hal2 {

enum class Facility {
  // Fundamental processor functionality.
  RCC_FACILITY,  // Note that RCC includes the primary oscillator as well.

  // Time-oriented facilities.
  OSCILLATOR_FACILITY,  // Secondary oscillators.
  SYSTICK_FACILITY,
  TIMER_FACILITY,
  CLOCK_FACILITY,

  // Random number generation.
  RNG_FACILITY,

  // Interactions via the pins of the processor.
  GPIO_FACILITY,

  // Watchdogs to verify behavior.
  WATCHDOG_FACILITY,

  // Analog-digital conversion.
  ADC_FACILITY,
  DAC_FACILITY,

  // Buses
  I2C_FACILITY,
  SPI_FACILITY,
  CAN_BUS_FACILITY,

  // Board-level peripherals.
  LED_FACILITY,
  PROGRAMMER_FACILITY,
  IMU_FACILITY,
  POWER_MONITOR_FACILITY,
  RF_RECEIVER_FACILITY,
  RF_TRANSMITTER_FACILITY,
};

struct DeviceId final {
  Facility facility;
  size_t index;

  constexpr bool operator==(const DeviceId&) const = default;
};

template <typename DeviceType, typename DriverType>
class Driver;

template <typename DeviceType, typename DriverType>
class Device {
 private:
  size_t ref_count_{};

  void inc_ref_count() noexcept {
    if (ref_count_++ == 0) {
      enable();
    }
  }

  void dec_ref_count() noexcept {
    if (--ref_count_ == 0) {
      disable();
    }
  }

  friend class Driver<DeviceType, DriverType>;

 protected:
  /**
   * Enable power and clock to this peripheral.
   */
  virtual void enable() noexcept = 0;

  /**
   * Disable power and clock to this peripheral.
   */
  virtual void disable() noexcept = 0;

 public:
  constexpr Device() noexcept = default;
  constexpr Device(const Device&) noexcept = default;
  constexpr Device(Device&&) noexcept = default;
  constexpr Device& operator=(const Device&) noexcept = default;
  constexpr Device& operator=(Device&&) noexcept = default;

  virtual ~Device() = default;

  // Access an interface to use the functionality of this peripheral.
  [[nodiscard]] DriverType access() { return DriverType{*reinterpret_cast<DeviceType*>(this)}; }
};

template <typename DeviceType, typename DriverType>
class Driver {
 protected:
  DeviceType* peripheral_{nullptr};

  constexpr Driver() noexcept = default;

  explicit Driver(DeviceType& peripheral) noexcept : peripheral_(&peripheral) {
    peripheral_->inc_ref_count();
  }

 public:
  virtual ~Driver() {
    if (peripheral_ != nullptr) {
      peripheral_->dec_ref_count();
    }
  }

  constexpr Driver(Driver&& rhs) noexcept : peripheral_(std::exchange(rhs.peripheral_, nullptr)) {}

  constexpr virtual Driver& operator=(Driver&& rhs) noexcept {
    std::swap(peripheral_, rhs.peripheral_);
    return *this;
  }

  // Create this Driver as another reference to the same peripheral.
  Driver(const Driver& rhs) noexcept : peripheral_(rhs.peripheral_) {
    if (peripheral_ != nullptr) {
      peripheral_->inc_ref_count();
    }
  }

  virtual Driver& operator=(const Driver& rhs) noexcept {
    if (peripheral_ != rhs.peripheral_) {
      if (peripheral_ != nullptr) {
        peripheral_->dec_ref_count();
      }
      peripheral_ = rhs.peripheral_;
      if (peripheral_ != nullptr) {
        peripheral_->inc_ref_count();
      }
    }
    return *this;
  }

  void ensure_valid(DeviceType& peripheral) noexcept {
    if (peripheral_ == nullptr) {
      peripheral_ = &peripheral;
      peripheral_->inc_ref_count();
    }
  }

  void invalidate() noexcept {
    if (peripheral_ != nullptr) {
      peripheral_->dec_ref_count();
    }
    peripheral_ = nullptr;
  }

  bool is_valid() const noexcept { return peripheral_ != nullptr; }
};

}  // namespace tvsc::hal2
