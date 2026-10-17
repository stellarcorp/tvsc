/**
 * Base classes for peripherals and their functions.
 *
 * The goal is to use RAII to manage power and clock to peripherals; if we aren't using the
 * peripheral's functions, it should be off. These mechanisms are important, since almost all of our
 * use cases will have severe power constraints, and turning off the power and clock whenever
 * possible should extend the lifetime of the satellite. To make this happen, we separate the
 * representation of the Peripheral itself from its function. Every Peripheral has a method to give
 * access to its functions. The base class for the functions, called Driver, increments a
 * reference count on instantiation. The Peripheral then watches the reference count to determine
 * when to enable clock and power to the peripheral and when to disable them.
 *
 * The overall structure hopefully makes it easy to implement new peripherals via subclassing. The
 * Peripheral base class represents a real peripheral on a board, especially one that can be enabled
 * or disabled, but without any access to the functionality of that peripheral. The Driver base
 * class represents access to the functions of the peripheral. The Driver provides an RAII
 * mechanism to control when the peripheral gets power and clock. To create a new peripheral, the
 * typical method will be to subclass both Peripheral and Driver, with the Peripheral acting as
 * the factory for the Driver subclass. The functionality of the peripheral should be exposed
 * only on the Driver subclass. The functionality may be implemented directly on the Driver
 * subclass, or it may delegate back to the Peripheral.
 *
 * In most of our peripherals, we will want to provide another layer between the interface of the
 * functionality and its implementation. This extra layer will allow us to simulate and test the
 * behavior of the satellite by providing fake implementations of the peripherals that work on
 * standard desktop- and server-class computers. These fake implementations will allow us to inject
 * faked sensor readings and validate controls.
 *
 * TODO(james): Peripheral subclasses should be able to contain references to other Peripherals and
 * the access() method should also guarantee that the dependency's power and clock are managed
 * correctly, likely via the same reference counting mechanism.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

namespace tvsc::hal2 {

template <typename PeripheralType, typename DriverType>
class Driver;

template <typename PeripheralType, typename DriverType>
class Peripheral {
 private:
  size_t ref_count_{};

  void inc_ref_count() {
    if (ref_count_++ == 0) {
      enable();
    }
  }

  void dec_ref_count() {
    if (--ref_count_ == 0) {
      disable();
    }
  }

  friend class Driver<PeripheralType, DriverType>;

 protected:
  /**
   * Enable power and clock to this peripheral.
   */
  virtual void enable() = 0;

  /**
   * Disable power and clock to this peripheral.
   */
  virtual void disable() = 0;

 public:
  constexpr Peripheral() noexcept = default;
  constexpr Peripheral(const Peripheral&) noexcept = default;
  constexpr Peripheral(Peripheral&&) noexcept = default;
  constexpr Peripheral& operator=(const Peripheral&) noexcept = default;
  constexpr Peripheral& operator=(Peripheral&&) noexcept = default;

  virtual ~Peripheral() = default;

  // Access an interface to use the functionality of this peripheral.
  [[nodiscard]] DriverType access() { return DriverType{*reinterpret_cast<PeripheralType*>(this)}; }
};

template <typename PeripheralType, typename DriverType>
class SingletonPeripheral : public Peripheral<PeripheralType, DriverType> {
 public:
  constexpr SingletonPeripheral() = default;
  SingletonPeripheral(const SingletonPeripheral&) = delete;
  SingletonPeripheral(SingletonPeripheral&&) = delete;

  virtual ~SingletonPeripheral() = default;
};

template <typename PeripheralType, typename DriverType>
class Driver {
 protected:
  PeripheralType* peripheral_{nullptr};

  constexpr Driver() noexcept = default;

  explicit Driver(PeripheralType& peripheral) : peripheral_(&peripheral) {
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
  Driver(const Driver& rhs) : peripheral_(rhs.peripheral_) {
    if (peripheral_ != nullptr) {
      peripheral_->inc_ref_count();
    }
  }

  virtual Driver& operator=(const Driver& rhs) {
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

  void ensure_valid(PeripheralType& peripheral) {
    if (peripheral_ == nullptr) {
      peripheral_ = &peripheral;
      peripheral_->inc_ref_count();
    }
  }

  void invalidate() {
    if (peripheral_ != nullptr) {
      peripheral_->dec_ref_count();
    }
    peripheral_ = nullptr;
  }

  bool is_valid() const { return peripheral_ != nullptr; }
};

}  // namespace tvsc::hal2
