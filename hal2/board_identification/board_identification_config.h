#pragma once

#include <cstddef>

#include "hal2/board_identification/board_identification.h"
#include "hal2/devices/device.h"
#include "hal2/devices/device_definitions.h"
#include "hal2/gpio/gpio.h"

namespace tvsc::hal2::board_identification {

template <typename DeviceRegistry, typename Board, size_t ADC_INDEX, gpio::PinRef POWER_PIN,
          gpio::PinRef SENSE_PIN>
class BoardIdentificationAccessor {
 private:
  static constexpr devices::DeviceId adc_id{devices::Facility::ADC_FACILITY, ADC_INDEX};
  static constexpr devices::DeviceId power_gpio_id{devices::Facility::GPIO_FACILITY,
                                                   POWER_PIN.port};
  static constexpr devices::DeviceId sense_gpio_id{devices::Facility::GPIO_FACILITY,
                                                   SENSE_PIN.port};

  BoardIdentificationDevice board_id_{devices::get_device<adc_id, DeviceRegistry>(*this),         //
                                      devices::get_device<power_gpio_id, DeviceRegistry>(*this),  //
                                      devices::get_device<sense_gpio_id, DeviceRegistry>(*this),  //
                                      POWER_PIN, SENSE_PIN};

 public:
  [[nodiscard]] auto& get() noexcept { return board_id_; }
  [[nodiscard]] auto& board_id() noexcept { return get(); }
};

}  // namespace tvsc::hal2::board_identification
