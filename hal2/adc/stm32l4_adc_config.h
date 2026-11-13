#pragma once

#include "hal2/adc/adc.h"
#include "hal2/adc/stm32l4_adc.h"
#include "hal2/config/device_definitions.h"
#include "hal2/device.h"

namespace tvsc::hal2::adc {

template <typename System, typename DeviceRegistry, size_t INDEX, size_t DMA_CONTROLLER_INDEX,
          uint8_t CHANNEL_INDEX>
class Stm32L4AdcAccessor;

template <typename System, typename DeviceRegistry, size_t DMA_CONTROLLER_INDEX,
          uint8_t CHANNEL_INDEX>
class Stm32L4AdcAccessor<System, DeviceRegistry, 0, DMA_CONTROLLER_INDEX, CHANNEL_INDEX> {
 private:
  static constexpr DeviceId DMA_CONTROLLER_ID{Facility::DMA_FACILITY, DMA_CONTROLLER_INDEX};
  using DmaControllerAccessor = config::accessor_type<System, DeviceRegistry, DMA_CONTROLLER_ID>;
  Stm32L4Adc adc_{0, static_cast<DmaControllerAccessor&>(static_cast<System&>(*this)).get(),
                  CHANNEL_INDEX};

 public:
  [[nodiscard]] AdcDevice& get() noexcept { return adc_; }
  [[nodiscard]] auto& adc0() noexcept { return get(); }
};

template <typename System, typename DeviceRegistry, size_t DMA_CONTROLLER_INDEX,
          uint8_t CHANNEL_INDEX>
class Stm32L4AdcAccessor<System, DeviceRegistry, 1, DMA_CONTROLLER_INDEX, CHANNEL_INDEX> {
 private:
  static constexpr DeviceId DMA_CONTROLLER_ID{Facility::DMA_FACILITY, DMA_CONTROLLER_INDEX};
  using DmaControllerAccessor = config::accessor_type<System, DeviceRegistry, DMA_CONTROLLER_ID>;
  Stm32L4Adc adc_{1, static_cast<DmaControllerAccessor&>(static_cast<System&>(*this)).get(),
                  CHANNEL_INDEX};

 public:
  [[nodiscard]] AdcDevice& get() noexcept { return adc_; }
  [[nodiscard]] auto& adc1() noexcept { return get(); }
};

}  // namespace tvsc::hal2::adc
