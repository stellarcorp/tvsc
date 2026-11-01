/**
 * Base classes for devices and their functions.
 *
 * The goal is to use RAII to manage power and clock to devices; if we aren't using the
 * device's functions, it should be off. These mechanisms are important, since almost all of our
 * use cases will have severe power constraints, and turning off the power and clock whenever
 * possible should extend the lifetime of the satellite. To make this happen, we separate the
 * representation of the Device itself from its function. Every Device has a method to give
 * access to its functions. The base class for the functions, called Driver, increments a
 * reference count on instantiation. The Device then watches the reference count to determine
 * when to enable clock and power to the device and when to disable them.
 *
 * The overall structure hopefully makes it easy to implement new devices via subclassing. The
 * Device base class represents a real device on a board, especially one that can be enabled
 * or disabled, but without any access to the functionality of that device. The Driver base
 * class represents access to the functions of the device. The Driver provides an RAII
 * mechanism to control when the device gets power and clock. To create a new device, the
 * typical method will be to subclass both Device and Driver, with the Device acting as
 * the factory for the Driver subclass. The functionality of the device should be exposed
 * only on the Driver subclass. The functionality may be implemented directly on the Driver
 * subclass, or it may delegate back to the Device.
 *
 * In most of our devices, we will want to provide another layer between the interface of the
 * functionality and its implementation. This extra layer will allow us to simulate and test the
 * behavior of the satellite by providing fake implementations of the devices that work on
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

  // Facilities for DMA set up. We include the DMA channels as their own facility to help assert
  // dependencies on the channels themselves.
  DMA_FACILITY,
  DMA_CHANNEL_FACILITY,

  // Analog-digital conversion.
  ADC_FACILITY,
  ADC_CHANNEL_FACILITY,
  DAC_FACILITY,
  DAC_CHANNEL_FACILITY,

  // Buses
  I2C_FACILITY,
  SPI_FACILITY,
  CAN_BUS_FACILITY,

  // Board-level devices.
  LED_FACILITY,
  PROGRAMMER_FACILITY,
  IMU_FACILITY,
  POWER_MONITOR_FACILITY,
  RF_RECEIVER_FACILITY,
  RF_TRANSMITTER_FACILITY,

  // Enum value for custom facilities.
  CUSTOM_FACILITY,
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
   * Enable power and clock to this device.
   */
  virtual void enable() noexcept = 0;

  /**
   * Disable power and clock to this device.
   */
  virtual void disable() noexcept = 0;

 public:
  constexpr Device() noexcept = default;
  constexpr Device(const Device&) noexcept = default;
  constexpr Device(Device&&) noexcept = default;
  constexpr Device& operator=(const Device&) noexcept = default;
  constexpr Device& operator=(Device&&) noexcept = default;

  virtual ~Device() = default;

  // Access a driver interface to use the functionality of this device.
  [[nodiscard]] DriverType access() { return DriverType{*reinterpret_cast<DeviceType*>(this)}; }
};

template <typename DeviceType, typename DriverType>
class Driver {
 protected:
  DeviceType* device_{nullptr};

  constexpr Driver() noexcept = default;

  explicit Driver(DeviceType& device) noexcept : device_(&device) { device_->inc_ref_count(); }

 public:
  virtual ~Driver() {
    if (device_ != nullptr) {
      device_->dec_ref_count();
    }
  }

  constexpr Driver(Driver&& rhs) noexcept : device_(std::exchange(rhs.device_, nullptr)) {}

  constexpr virtual Driver& operator=(Driver&& rhs) noexcept {
    std::swap(device_, rhs.device_);
    return *this;
  }

  // Create this Driver as another reference to the same device.
  Driver(const Driver& rhs) noexcept : device_(rhs.device_) {
    if (device_ != nullptr) {
      device_->inc_ref_count();
    }
  }

  virtual Driver& operator=(const Driver& rhs) noexcept {
    if (device_ != rhs.device_) {
      if (device_ != nullptr) {
        device_->dec_ref_count();
      }
      device_ = rhs.device_;
      if (device_ != nullptr) {
        device_->inc_ref_count();
      }
    }
    return *this;
  }

  void ensure_valid(DeviceType& device) noexcept {
    if (device_ == nullptr) {
      device_ = &device;
      device_->inc_ref_count();
    }
  }

  void invalidate() noexcept {
    if (device_ != nullptr) {
      device_->dec_ref_count();
    }
    device_ = nullptr;
  }

  bool is_valid() const noexcept { return device_ != nullptr; }
};

}  // namespace tvsc::hal2
