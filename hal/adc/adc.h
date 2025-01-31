#pragma once

#include <cstdint>
#include <cstdlib>

#include "hal/gpio/gpio.h"

namespace tvsc::hal::adc {

class Adc {
 public:
  /**
   * Measure the voltage on a pin, optionally using an opamp to amplify the voltage by a gain.
   *
   * Configures an ADC to measure the voltage on the given pin. If the gain is non-unity, the
   * onboard opamp is configured to amplify the signal before conversion. The measurement is
   * asynchronous; the is_running() method will return false when the measurement is complete. The
   * result is stored in the location specified by destination.
   *
   * TODO(james): Consider adding a callback. The callback could be on this method directly or it
   * could be on the Adc instance as a whole.
   */
  virtual void start_conversion(gpio::PortPin pin, uint32_t* destination,
                                size_t destination_buffer_size) = 0;

  virtual void calibrate_single_ended_input() = 0;
  virtual void calibrate_differential_input() = 0;
  virtual uint32_t read_calibration_factor() = 0;
  virtual void write_calibration_factor(uint32_t factor) = 0;

  virtual bool is_running() = 0;

  /**
   * Stop the current measurement or calibration sequence.
   */
  virtual void stop() = 0;
};

}  // namespace tvsc::hal::adc
