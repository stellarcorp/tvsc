#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

namespace tvsc::hal::dac {

class Dac;

class DacPeripheral {
 private:
  size_t ref_count_{};

 protected:
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

  virtual void enable() = 0;
  virtual void disable() = 0;

  virtual void set_value(uint32_t value, uint8_t channel) = 0;
  virtual void clear_value(uint8_t channel) = 0;

  virtual void set_resolution(uint8_t bits_resolution, uint8_t channel) = 0;

  friend class Dac;

 public:
  virtual ~DacPeripheral() = default;

  // Use the functionality of this peripheral. Power and clock will be enabled.
  Dac access();
};

class Dac final {
 private:
  DacPeripheral* peripheral_;

  explicit Dac(DacPeripheral& peripheral) : peripheral_(&peripheral) {
    peripheral_->inc_ref_count();
  }

  friend class DacPeripheral;

 public:
  ~Dac() {
    if (peripheral_ != nullptr) {
      peripheral_->dec_ref_count();
    }
  }

  Dac(Dac&& rhs) : peripheral_(std::exchange(rhs.peripheral_, nullptr)) {}

  Dac& operator=(Dac&& rhs) {
    std::swap(peripheral_, rhs.peripheral_);
    return *this;
  }

  Dac(const Dac& rhs) : peripheral_(rhs.peripheral_) { peripheral_->inc_ref_count(); }

  Dac& operator=(const Dac& rhs) {
    peripheral_ = rhs.peripheral_;
    peripheral_->inc_ref_count();
    return *this;
  }

  void set_value(uint32_t value, uint8_t channel = 0) { peripheral_->set_value(value, channel); }
  void clear_value(uint8_t channel = 0) { peripheral_->clear_value(channel); }

  void set_resolution(uint8_t bits_resolution, uint8_t channel = 0) {
    peripheral_->set_resolution(bits_resolution, channel);
  }

  // Turn on power and clock to this peripheral.
  Dac access();
};

}  // namespace tvsc::hal::dac
