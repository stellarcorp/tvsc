#pragma once

#include <array>
#include <cstdint>
#include <cstdlib>

#include "hal/enable_lock.h"
#include "hal/gpio/gpio.h"
#include "hal/timer/timer.h"

namespace tvsc::hal::adc {

class Adc {
 public:
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

  virtual void reset_after_conversion() = 0;

  virtual void set_resolution(uint8_t bits_resolution) = 0;

  virtual void calibrate_single_ended_input() = 0;
  virtual void calibrate_differential_input() = 0;
  virtual uint32_t read_calibration_factor() = 0;
  virtual void write_calibration_factor(uint32_t factor) = 0;

  virtual bool is_running() = 0;

  /**
   * Stop the current measurement sequence.
   */
  virtual void stop() = 0;

  virtual void handle_interrupt() = 0;

  // Turn on power and clock to this peripheral.
  virtual EnableLock enable() = 0;
};

}  // namespace tvsc::hal::adc
