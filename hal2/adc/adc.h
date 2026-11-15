#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <span>

#include "buffer/buffer_allocator.h"
#include "hal2/device.h"
#include "hal2/gpio/gpio.h"
#include "hal2/timer/timer.h"

namespace tvsc::hal2::adc {

using namespace std::chrono_literals;

class AdcSessionConfiguration final {
 public:
  enum class Resolution : uint8_t {
    Bits6,
    Bits8,
    Bits10,
    Bits12,
    Bits14,
    Bits16,
  };

  enum class Alignment : uint8_t {
    Right,
    Left,
  };

  enum class StreamMode : uint8_t {
    SingleScan,
    ContinuousScan,
  };

  enum class SampleCycles : uint8_t {
    Cycles2,
    Cycles6,
    Cycles12,
    Cycles24,
    Cycles47,
    Cycles92,
    Cycles247,
    Cycles640,
  };
  static constexpr SampleCycles DEFAULT_SAMPLE_CYCLES{SampleCycles::Cycles24};

  enum class TriggerEdge : uint8_t {
    Rising,
    Falling,
    Both,
  };

  std::array<gpio::PinRef, 16> pins{};
  size_t pin_count{0};
  buffer::BufferRef buffer{nullptr, 0};
  // TODO(james): Placeholder. Triggering conversions with a timer is not yet supported.
  timer::Timer* timer{nullptr};
  Resolution resolution{Resolution::Bits12};
  Alignment alignment{Alignment::Right};
  StreamMode stream_mode{StreamMode::SingleScan};
  TriggerEdge trigger_edge{TriggerEdge::Rising};
  SampleCycles sample_cycles{DEFAULT_SAMPLE_CYCLES};

  constexpr AdcSessionConfiguration& include_pin(gpio::PinRef pin) noexcept {
    pins.at(pin_count++) = pin;
    return *this;
  }

  constexpr AdcSessionConfiguration& include_pin_sequence(
      std::span<const gpio::PinRef> pins) noexcept {
    for (auto p : pins) {
      include_pin(p);
    }
    return *this;
  }

  [[nodiscard]] constexpr std::span<const gpio::PinRef> pin_sequence() const noexcept {
    return std::span{pins}.subspan(0, pin_count);
  }

  [[nodiscard]] constexpr uint32_t bytes_per_sample() const noexcept {
    switch (resolution) {
      case AdcSessionConfiguration::Resolution::Bits6:
        // Fall-through deliberate.
      case AdcSessionConfiguration::Resolution::Bits8:
        return 1;
      case AdcSessionConfiguration::Resolution::Bits10:
      // Fall-through deliberate.
      case AdcSessionConfiguration::Resolution::Bits12:
      // Fall-through deliberate.
      case AdcSessionConfiguration::Resolution::Bits14:
      // Fall-through deliberate.
      case AdcSessionConfiguration::Resolution::Bits16:
        return 2;
    }
  }
};

class Adc;

class AdcDevice : public Device<AdcDevice, Adc> {
 private:
  virtual void enable() noexcept = 0;
  virtual void disable() noexcept = 0;

  virtual void start(const AdcSessionConfiguration&) noexcept = 0;
  [[nodiscard]] virtual bool is_running() noexcept = 0;
  virtual void stop() noexcept = 0;

  /**
   * Measure the voltage on a pin. This method blocks until the conversion is complete.
   */
  [[nodiscard]] virtual uint16_t measure_value(
      gpio::PinRef pin, AdcSessionConfiguration::SampleCycles sample_time =
                            AdcSessionConfiguration::DEFAULT_SAMPLE_CYCLES) noexcept = 0;

  virtual void calibrate_single_ended_input() noexcept = 0;
  virtual void calibrate_differential_input() noexcept = 0;
  [[nodiscard]] virtual uint32_t read_calibration_factor() noexcept = 0;
  virtual void write_calibration_factor(uint32_t factor) noexcept = 0;

  friend class Adc;

 public:
  virtual ~AdcDevice() = default;
};

class Adc final : public Driver<AdcDevice, Adc> {
 protected:
  explicit Adc(AdcDevice& device) noexcept : Driver<AdcDevice, Adc>(device) {}

  friend class Device<AdcDevice, Adc>;

 public:
  Adc() noexcept = default;

  void start(const AdcSessionConfiguration& config) noexcept { device_->start(config); }

  [[nodiscard]] bool is_running() noexcept { return device_->is_running(); }

  void stop() noexcept { device_->stop(); }

  /**
   * Convenience method that makes a single measurement and returns the value. This method
   * blocks while the measurement is being taken.
   */
  [[nodiscard]] uint16_t measure_value(
      gpio::PinRef pin, AdcSessionConfiguration::SampleCycles sample_time =
                            AdcSessionConfiguration::DEFAULT_SAMPLE_CYCLES) noexcept {
    return device_->measure_value(pin, sample_time);
  }

  void calibrate_single_ended_input() noexcept { device_->calibrate_single_ended_input(); }

  void calibrate_differential_input() noexcept { device_->calibrate_differential_input(); }

  [[nodiscard]] uint32_t read_calibration_factor() noexcept {
    return device_->read_calibration_factor();
  }

  void write_calibration_factor(uint32_t factor) noexcept {
    device_->write_calibration_factor(factor);
  }
};

}  // namespace tvsc::hal2::adc
