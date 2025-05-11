#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>

#include "hal/gpio/gpio.h"
#include "hal/peripheral.h"
#include "hal/timer/timer.h"

namespace tvsc::hal::adc {

using namespace std::chrono_literals;

class Adc;

class AdcPeripheral : public Peripheral<AdcPeripheral, Adc> {
 private:
  virtual void enable() = 0;
  virtual void disable() = 0;

  /**
   * Measure the voltage on a pin.
   *
   * Configures an ADC to measure the voltage on the given pin. The measurement is
   * asynchronous; the is_running() method will return false when the measurement is complete. The
   * result is stored in the location specified by destination using DMA.
   *
   * TODO(james): Rework this API to support converting multiple pins at one time. Likely, we would
   * want begin/end iterators as parameters. (Note: can't use std::array as it would require
   * templating on the size of the array, and template functions can't be virtual.
   */
  virtual void start_single_conversion(gpio::PortPin pin, uint32_t* destination,
                                       size_t destination_buffer_size) = 0;

  virtual void start_conversion_stream(gpio::PortPin pin, uint32_t* destination,
                                       size_t destination_buffer_size, timer::Timer& trigger) = 0;

  /**
   * Measure the voltage on a pin. This method blocks until the conversion is complete.
   */
  virtual uint16_t measure_value(gpio::PortPin pin, std::chrono::milliseconds timeout) = 0;

  virtual void reset_after_conversion() = 0;

  virtual void set_resolution(uint8_t bits_resolution) = 0;

  // Methods to configure data alignment.
  virtual void use_data_align_left() = 0;
  virtual void use_data_align_right() = 0;

  virtual void calibrate_single_ended_input() = 0;
  virtual void calibrate_differential_input() = 0;
  virtual uint32_t read_calibration_factor() = 0;
  virtual void write_calibration_factor(uint32_t factor) = 0;

  virtual bool is_running() = 0;

  /**
   * Stop the current measurement sequence.
   */
  virtual void stop() = 0;

  friend class Adc;

 public:
  virtual ~AdcPeripheral() = default;

  virtual void handle_interrupt() = 0;
};

class Adc final : public Functional<AdcPeripheral, Adc> {
 protected:
  explicit Adc(AdcPeripheral& peripheral) : Functional<AdcPeripheral, Adc>(peripheral) {}

  friend class Peripheral<AdcPeripheral, Adc>;

 public:
  void start_single_conversion(gpio::PortPin pin, uint32_t* destination,
                               size_t destination_buffer_size) {
    peripheral_->start_single_conversion(pin, destination, destination_buffer_size);
  }

  void start_conversion_stream(gpio::PortPin pin, uint32_t* destination,
                               size_t destination_buffer_size, timer::Timer& trigger) {
    peripheral_->start_conversion_stream(pin, destination, destination_buffer_size, trigger);
  }

  uint16_t measure_value(gpio::PortPin pin, std::chrono::milliseconds timeout = 10ms) {
    return peripheral_->measure_value(pin, timeout);
  }

  void reset_after_conversion() { peripheral_->reset_after_conversion(); }

  void set_resolution(uint8_t bits_resolution) { peripheral_->set_resolution(bits_resolution); }

  void use_data_align_left() { peripheral_->use_data_align_left(); }
  void use_data_align_right() { peripheral_->use_data_align_right(); }

  void calibrate_single_ended_input() { peripheral_->calibrate_single_ended_input(); }
  void calibrate_differential_input() { peripheral_->calibrate_differential_input(); }
  uint32_t read_calibration_factor() { return peripheral_->read_calibration_factor(); }
  void write_calibration_factor(uint32_t factor) { peripheral_->write_calibration_factor(factor); }

  bool is_running() { return peripheral_->is_running(); }

  /**
   * Stop the current measurement sequence.
   */
  void stop() { peripheral_->stop(); }
};

}  // namespace tvsc::hal::adc
