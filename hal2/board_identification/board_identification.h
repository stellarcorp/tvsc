#pragma once

#include "hal2/adc/adc.h"
#include "hal2/board_identification/board_ids.h"
#include "hal2/board_identification/voltage_divider.h"
#include "hal2/clock/clock.h"
#include "hal2/device.h"
#include "hal2/gpio/gpio.h"

namespace tvsc::hal2::board_identification {

class BoardIdentificationDriver;

class BoardIdentificationDevice final
    : public Device<BoardIdentificationDevice, BoardIdentificationDriver> {
 private:
  adc::AdcDevice* adc_device_;
  gpio::PinDevice power_pin_device_;
  gpio::PinDevice sense_pin_device_;
  adc::Adc adc_{};
  gpio::Pin power_pin_{};
  gpio::Pin sense_pin_{};

  void enable() noexcept override;
  void disable() noexcept override;

  void start() noexcept;
  [[nodiscard]] bool is_running() noexcept;
  void stop() noexcept;

  template <clock::SleepStrategy SLEEP_STRATEGY>
  [[nodiscard]] BoardId read_board_id(clock::Clock<SLEEP_STRATEGY>& clock) noexcept {
    power_pin_.on();

    BoardId result{cast_to_underlying_type(CanonicalBoardIds::UNKNOWN)};
    const uint16_t measured_value{adc_.measure_value(sense_pin_.ref())};

    power_pin_.off();

    result = determine_board_id(measured_value);
    return result;
  }

  friend class BoardIdentificationDriver;

 public:
  BoardIdentificationDevice(adc::AdcDevice& adc_device, gpio::GpioDevice& power_gpio_bank,
                            gpio::GpioDevice& sense_gpio_bank, gpio::PinRef power_pin,
                            gpio::PinRef sense_pin) noexcept
      : adc_device_(&adc_device),
        power_pin_device_(gpio::PinDevice{power_gpio_bank, power_pin}),
        sense_pin_device_(gpio::PinDevice{sense_gpio_bank, sense_pin}) {}
};

class BoardIdentificationDriver final
    : public Driver<BoardIdentificationDevice, BoardIdentificationDriver> {
 private:
  explicit BoardIdentificationDriver(BoardIdentificationDevice& device) noexcept
      : Driver<BoardIdentificationDevice, BoardIdentificationDriver>(device) {}

  friend Device<BoardIdentificationDevice, BoardIdentificationDriver>;

 public:
  void start() noexcept { device_->start(); }
  [[nodiscard]] bool is_running() noexcept { return device_->is_running(); }
  void stop() noexcept { device_->stop(); }

  template <clock::SleepStrategy SLEEP_STRATEGY>
  [[nodiscard]] BoardId read_board_id(clock::Clock<SLEEP_STRATEGY>& clock) noexcept {
    return device_->read_board_id(clock);
  }
};

}  // namespace tvsc::hal2::board_identification
