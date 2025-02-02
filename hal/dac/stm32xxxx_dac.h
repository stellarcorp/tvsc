#pragma once

#include <array>
#include <cstdint>

#include "hal/dac/dac.h"
#include "hal/power_token.h"
#include "hal/register.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::dac {

template <uint8_t NUM_CHANNELS = 1>
class DacStm32xxxx final : public Dac {
 private:
  struct Channel final {
    uint32_t channel_id;
    DAC_ChannelConfTypeDef config;
    uint32_t hal_resolution_id{DAC_ALIGN_12B_R};
    uint8_t bits_resolution{12};
  };

  DAC_HandleTypeDef hdac_{};
  std::array<Channel, NUM_CHANNELS> channels_;
  uint16_t use_counter_{0};

  void turn_off() { __HAL_RCC_DAC1_CLK_DISABLE(); }

 public:
  DacStm32xxxx(DAC_TypeDef* hal_dac) {
    hdac_.Instance = hal_dac;
    HAL_DAC_Init(&hdac_);

    if constexpr (NUM_CHANNELS >= 1) {
#if defined(DAC_CHANNEL_1)
      channels_[0].channel_id = DAC_CHANNEL_1;
#endif
    }
    if constexpr (NUM_CHANNELS >= 2) {
#if defined(DAC_CHANNEL_2)
      channels_[1].channel_id = DAC_CHANNEL_2;
#endif
    }
    if constexpr (NUM_CHANNELS >= 2) {
      static_assert(NUM_CHANNELS <= 2, "Implement DAC channels beyond 2");
    }
    for (auto& channel : channels_) {
      channel.config.DAC_Trigger = DAC_TRIGGER_NONE;  // No trigger, direct output
      channel.config.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    }
  }

  void set_value(uint32_t value, uint8_t channel) override {
    Channel& channel_config{channels_.at(channel)};
    // Clamp the value according to the bits of resolution requested.
    if (value > (1UL << channel_config.bits_resolution) - 1) {
      value = (1UL << channel_config.bits_resolution) - 1;
    }
    HAL_DAC_SetValue(&hdac_, channel_config.channel_id, channel_config.hal_resolution_id, value);
    HAL_DAC_Start(&hdac_, channel_config.channel_id);
  }

  void clear_value(uint8_t channel) override {
    // This either turns off the DAC, letting the voltage on the pin float, or leaves the DAC
    // presenting the last voltage set.
    const auto channel_id{channels_.at(channel).channel_id};
    HAL_DAC_Stop(&hdac_, channel_id);
  }

  void set_resolution(uint8_t bits_resolution, uint8_t channel) override {
    if (bits_resolution <= 8) {
      channels_.at(channel).hal_resolution_id = DAC_ALIGN_8B_R;
      channels_.at(channel).bits_resolution = bits_resolution;
    } else if (bits_resolution <= 12) {
      channels_.at(channel).hal_resolution_id = DAC_ALIGN_12B_R;
      channels_.at(channel).bits_resolution = bits_resolution;
    } else if (bits_resolution > 12) {
      // Clamp the resolution. The value register only accepts 16-bit values. Any more resolution
      // would cause a loss of MSB bits.
      if (bits_resolution > 16) {
        bits_resolution = 16;
      }
      channels_.at(channel).hal_resolution_id = DAC_ALIGN_12B_L;
      channels_.at(channel).bits_resolution = bits_resolution;
    }
  }

  PowerToken turn_on() {
    if (use_counter_ == 0) {
      __HAL_RCC_DAC1_CLK_ENABLE();
    }
    ++use_counter_;
    return PowerToken([this]() {
      --use_counter_;
      if (use_counter_ == 0) {
        turn_off();
      }
    });
  }
};

}  // namespace tvsc::hal::dac
