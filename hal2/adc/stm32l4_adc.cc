#include "hal2/adc/stm32l4_adc.h"

#include <array>
#include <chrono>
#include <cstdint>

#include "base/except.h"
#include "hal2/error.h"
#include "hal2/gpio/gpio.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::adc {

// Cache of the last measured values for each ADC.
#if defined(ADC3)
DEBUG_VAR std::array<uint16_t, 3> adc_measured_values{};
#elif defined(ADC2)
DEBUG_VAR std::array<uint16_t, 2> adc_measured_values{};
#else
DEBUG_VAR std::array<uint16_t, 1> adc_measured_values{};
#endif

[[nodiscard]] constexpr inline uint32_t get_channel(uint8_t adc_index, gpio::PinRef pin) noexcept {
  switch (adc_index) {
    // ADC1
    case 0:
      switch (pin.port) {
        case 0:  // Port A
          switch (pin.pin) {
            case 0:
              return ADC_CHANNEL_5;  // PA0 -> ADC1_IN5
            case 1:
              return ADC_CHANNEL_6;  // PA1 -> ADC1_IN6
            case 2:
              return ADC_CHANNEL_7;  // PA2 -> ADC1_IN7
            case 3:
              return ADC_CHANNEL_8;  // PA3 -> ADC1_IN8
            case 4:
              return ADC_CHANNEL_9;  // PA4 -> ADC1_IN9
            case 5:
              return ADC_CHANNEL_10;  // PA5 -> ADC1_IN10
            case 6:
              return ADC_CHANNEL_11;  // PA6 -> ADC1_IN11
            case 7:
              return ADC_CHANNEL_12;  // PA7 -> ADC1_IN12
          }
          break;
        case 1:  // Port B
          switch (pin.pin) {
            case 0:
              return ADC_CHANNEL_15;  // PB0 -> ADC1_IN15
            case 1:
              return ADC_CHANNEL_16;  // PB1 -> ADC1_IN16
          }
          break;
        case 2:  // Port C
          switch (pin.pin) {
            case 0:
              return ADC_CHANNEL_1;  // PC0 -> ADC1_IN1
            case 1:
              return ADC_CHANNEL_2;  // PC1 -> ADC1_IN2
            case 2:
              return ADC_CHANNEL_3;  // PC2 -> ADC1_IN3
            case 3:
              return ADC_CHANNEL_4;  // PC3 -> ADC1_IN4
            case 4:
              return ADC_CHANNEL_13;  // PC4 -> ADC1_IN13
            case 5:
              return ADC_CHANNEL_14;  // PC5 -> ADC1_IN14
          }
          break;
      }
      break;

      // ADC2
    case 1:
      switch (pin.port) {
        case 0:  // Port A
          switch (pin.pin) {
            case 0:
              return ADC_CHANNEL_5;  // PA0 -> ADC2_IN5
            case 1:
              return ADC_CHANNEL_6;  // PA1 -> ADC2_IN6
            case 2:
              return ADC_CHANNEL_7;  // PA2 -> ADC2_IN7
            case 3:
              return ADC_CHANNEL_8;  // PA3 -> ADC2_IN8
            case 4:
              return ADC_CHANNEL_9;  // PA4 -> ADC2_IN9
            case 5:
              return ADC_CHANNEL_10;  // PA5 -> ADC2_IN10
            case 6:
              return ADC_CHANNEL_11;  // PA6 -> ADC2_IN11
            case 7:
              return ADC_CHANNEL_12;  // PA7 -> ADC2_IN12
          }
          break;
        case 1:  // Port B
          switch (pin.pin) {
            case 0:
              return ADC_CHANNEL_15;  // PB0 -> ADC2_IN15
            case 1:
              return ADC_CHANNEL_16;  // PB1 -> ADC2_IN16
          }
          break;
        case 2:  // Port C
          switch (pin.pin) {
            case 0:
              return ADC_CHANNEL_1;  // PC0 -> ADC2_IN1
            case 1:
              return ADC_CHANNEL_2;  // PC1 -> ADC2_IN2
            case 2:
              return ADC_CHANNEL_3;  // PC2 -> ADC2_IN3
            case 3:
              return ADC_CHANNEL_4;  // PC3 -> ADC2_IN4
            case 4:
              return ADC_CHANNEL_13;  // PC4 -> ADC2_IN13
            case 5:
              return ADC_CHANNEL_14;  // PC5 -> ADC2_IN14
          }
          break;
      }
      break;

      // ADC3
    case 2:
      switch (pin.port) {
        case 2:  // Port C (ADC3 shares lower PC channels on high-density lines)
          switch (pin.pin) {
            case 0:
              return ADC_CHANNEL_1;  // PC0 -> ADC3_IN1
            case 1:
              return ADC_CHANNEL_2;  // PC1 -> ADC3_IN2
            case 2:
              return ADC_CHANNEL_3;  // PC3 -> ADC3_IN3
            case 3:
              return ADC_CHANNEL_4;  // PC3 -> ADC3_IN4
          }
          break;
        case 5:  // Port F (Extended ADC3 inputs on larger STM32L4/L4+ packages)
          switch (pin.pin) {
            case 3:
              return ADC_CHANNEL_6;  // PF3 -> ADC3_IN6
            case 4:
              return ADC_CHANNEL_9;  // PF4 -> ADC3_IN9
            case 5:
              return ADC_CHANNEL_10;  // PF5 -> ADC3_IN10
            case 6:
              return ADC_CHANNEL_4;  // PF6 -> ADC3_IN4
            case 7:
              return ADC_CHANNEL_5;  // PF7 -> ADC3_IN5
            case 8:
              return ADC_CHANNEL_6;  // PF8 -> ADC3_IN6
            case 9:
              return ADC_CHANNEL_7;  // PF9 -> ADC3_IN7
            case 10:
              return ADC_CHANNEL_8;  // PF10 -> ADC3_IN8
          }
          break;
      }
      break;
  }

  error();
}

[[nodiscard]] constexpr inline uint32_t to_stm32_sample_time(
    AdcSessionConfiguration::SampleCycles cycles) noexcept {
  switch (cycles) {
    case AdcSessionConfiguration::SampleCycles::Cycles2:
      return ADC_SAMPLETIME_2CYCLES_5;
    case AdcSessionConfiguration::SampleCycles::Cycles6:
      return ADC_SAMPLETIME_6CYCLES_5;
    case AdcSessionConfiguration::SampleCycles::Cycles12:
      return ADC_SAMPLETIME_12CYCLES_5;
    case AdcSessionConfiguration::SampleCycles::Cycles24:
      return ADC_SAMPLETIME_24CYCLES_5;
    case AdcSessionConfiguration::SampleCycles::Cycles47:
      return ADC_SAMPLETIME_47CYCLES_5;
    case AdcSessionConfiguration::SampleCycles::Cycles92:
      return ADC_SAMPLETIME_92CYCLES_5;
    case AdcSessionConfiguration::SampleCycles::Cycles247:
      return ADC_SAMPLETIME_247CYCLES_5;
    case AdcSessionConfiguration::SampleCycles::Cycles640:
      return ADC_SAMPLETIME_640CYCLES_5;
  }
  error();
}

[[nodiscard]] constexpr inline dma::DmaSessionConfiguration extract_dma_config(
    const AdcSessionConfiguration& config, uint8_t adc_index) noexcept {
  dma::DmaSessionConfiguration dma_config{};

  dma::DmaSessionConfiguration::TransferWidth transfer_width{
      dma::DmaSessionConfiguration::TransferWidth::BYTE};
  if (config.bytes_per_sample() == 1) {
    transfer_width = dma::DmaSessionConfiguration::TransferWidth::BYTE;
  } else {
    transfer_width = dma::DmaSessionConfiguration::TransferWidth::HALF_WORD;
  }

  dma_config.peripheral_data_width = transfer_width;
  dma_config.memory_data_width = transfer_width;

  dma_config.direction = dma::DmaSessionConfiguration::Direction::FROM_PERIPHERAL;

  switch (config.stream_mode) {
    case AdcSessionConfiguration::StreamMode::SingleScan:
      dma_config.stream_mode = dma::DmaSessionConfiguration::StreamMode::SINGLE_SHOT;
      break;
    case AdcSessionConfiguration::StreamMode::ContinuousScan:
      dma_config.stream_mode = dma::DmaSessionConfiguration::StreamMode::CONTINUOUS;
      break;
  }

  dma_config.peripheral_device = DeviceId{Facility::ADC_FACILITY, adc_index};
  dma_config.priority = dma::DmaSessionConfiguration::Priority::MEDIUM;

  return dma_config;
}

inline void configure_dma(dma::DmaChannel& dma_channel, const AdcSessionConfiguration& config,
                          uint8_t adc_index) noexcept {
  dma_channel.configure(extract_dma_config(config, adc_index));

  HAL_DMA_Init(dma_channel.implementation_handle<DMA_HandleTypeDef>());
}

inline void configure_adc(ADC_HandleTypeDef& adc_handle,
                          const AdcSessionConfiguration& config) noexcept {
  adc_handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;

  switch (config.resolution) {
    case AdcSessionConfiguration::Resolution::Bits6:
      adc_handle.Init.Resolution = ADC_RESOLUTION_6B;
      break;
    case AdcSessionConfiguration::Resolution::Bits8:
      adc_handle.Init.Resolution = ADC_RESOLUTION_8B;
      break;
    case AdcSessionConfiguration::Resolution::Bits10:
      adc_handle.Init.Resolution = ADC_RESOLUTION_10B;
      break;
    case AdcSessionConfiguration::Resolution::Bits12:
    default:
      adc_handle.Init.Resolution = ADC_RESOLUTION_12B;
      break;
  }

  if (config.alignment == AdcSessionConfiguration::Alignment::Right) {
    adc_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  } else {
    adc_handle.Init.DataAlign = ADC_DATAALIGN_LEFT;
  }

  // TODO(james): Determine how this setting effects the sampling.
  adc_handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;

  if (config.pin_count > 1) {
    adc_handle.Init.ScanConvMode = ADC_SCAN_ENABLE;
  } else {
    adc_handle.Init.ScanConvMode = ADC_SCAN_DISABLE;
  }

  // Set when the EOC (end of conversion flag) goes high. This setting causes it to pulse high at
  // the end of the sequence of conversions (all of the pins), rather than for each individual
  // conversion.
  adc_handle.Init.EOCSelection = ADC_EOC_SEQ_CONV;

  // TODO(james): Determine how this setting effects the sampling.
  adc_handle.Init.LowPowerAutoWait = DISABLE;

  if (config.stream_mode == AdcSessionConfiguration::StreamMode::ContinuousScan) {
    adc_handle.Init.ContinuousConvMode = ENABLE;
    adc_handle.Init.DMAContinuousRequests = ENABLE;
  } else {
    adc_handle.Init.ContinuousConvMode = DISABLE;
    adc_handle.Init.DMAContinuousRequests = DISABLE;
  }

  adc_handle.Init.NbrOfConversion = static_cast<uint32_t>(config.pin_count);

  // Disable discontinuous conversion mode.
  // This setting determines whether the ordered list of pins (the conversion sequence) is scanned
  // all at once on a single trigger, or broken up into smaller pieces across multiple triggers. We
  // do not currently support or need the functionality to use multiple triggers.
  adc_handle.Init.DiscontinuousConvMode = DISABLE;

  // Configure external trigger source and edge if a timer is present
  if (config.timer != nullptr) {
    except<std::domain_error>("Triggering an ADC conversion with a timer is unimplemented");
    /*
    // Assuming a helper function trigger_flag exists for mapping timer instances/events
    adc_handle.Init.ExternalTrigConv = trigger_flag(*config.timer);

    switch (config.trigger_edge) {
      case AdcSessionConfiguration::TriggerEdge::Falling:
        adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_FALLING;
        break;
      case AdcSessionConfiguration::TriggerEdge::Both:
        adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISINGFALLING;
        break;
      case AdcSessionConfiguration::TriggerEdge::Rising:
      default:
        adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
        break;
    }
    */
  } else {
    adc_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  }

  HAL_ADC_Init(&adc_handle);
}

inline void configure_adc_channel(ADC_HandleTypeDef& adc_handle, uint8_t adc_index,
                                  const AdcSessionConfiguration& config, uint8_t index) noexcept {
  ADC_ChannelConfTypeDef channel_config{};
  channel_config.Rank = index + 1;
  gpio::PinRef pin{config.pins[index]};
  channel_config.Channel = get_channel(adc_index, pin);

  // Map AdcSessionConfiguration::SampleCycles to STM32 HAL sample time constants
  // Note: Adjust the mapping targets to match your specific target header definitions.
  // Using Cycles28 as a standard default baseline mapping if needed.
  channel_config.SamplingTime = to_stm32_sample_time(config.sample_cycles);

  channel_config.SingleDiff = ADC_SINGLE_ENDED;

  // These settings offset the sample value by a fixed amount. Disabled for this design.
  channel_config.OffsetNumber = ADC_OFFSET_NONE;
  channel_config.Offset = 0;

  HAL_ADC_ConfigChannel(&adc_handle, &channel_config);
}

void Stm32L4Adc::start(const AdcSessionConfiguration& config) noexcept {
  stop();
  if (!dma_channel_.is_valid()) {
    dma_channel_ = dma_controller_->acquire_channel(dma_channel_index_);
  }

  configure_dma(dma_channel_, config, adc_index_);

  configure_adc(adc_, config);

  for (uint32_t i = 0; i < config.pin_count; ++i) {
    configure_adc_channel(adc_, adc_index_, config, i);
  }

  // Link DMA to ADC1. This allows the DMA's interrupt handler to (eventually) call the ADC's
  // interrupt handler, among other things.
  DMA_HandleTypeDef* dma_handle{dma_channel_.implementation_handle<DMA_HandleTypeDef>()};
  __HAL_LINKDMA(&adc_, DMA_Handle, *dma_handle);

  const auto& dest{config.buffer};
  HAL_ADC_Start_DMA(&adc_, reinterpret_cast<uint32_t*>(dest.address),
                    dest.size / config.bytes_per_sample());
}

[[nodiscard]] uint16_t Stm32L4Adc::measure_value(
    gpio::PinRef pin, AdcSessionConfiguration::SampleCycles sample_time) noexcept {
  stop();

  adc_.Init.Resolution = ADC_RESOLUTION_12B;
  // TODO(james): Determine how this setting effects the sampling.
  adc_.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  adc_.Init.ScanConvMode = ADC_SCAN_DISABLE;
  adc_.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  adc_.Init.LowPowerAutoWait = DISABLE;
  adc_.Init.ContinuousConvMode = DISABLE;
  adc_.Init.NbrOfConversion = 1;
  adc_.Init.DiscontinuousConvMode = DISABLE;
  adc_.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  adc_.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  adc_.Init.DMAContinuousRequests = DISABLE;
  adc_.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;

  // Configure an ADC Channel for each pin.
  ADC_ChannelConfTypeDef channel_config{};
  channel_config.Channel = get_channel(adc_index_, pin);
  channel_config.Rank = ADC_REGULAR_RANK_1;
  channel_config.SamplingTime = to_stm32_sample_time(sample_time);
  channel_config.SingleDiff = ADC_SINGLE_ENDED;
  channel_config.OffsetNumber = ADC_OFFSET_NONE;
  channel_config.Offset = 0;

  if (HAL_ADC_Init(&adc_) != HAL_OK) {
    except<std::logic_error>("Invalid configuration being passed to the ADC");
  }
  if (HAL_ADC_ConfigChannel(&adc_, &channel_config) != HAL_OK) {
    except<std::logic_error>("Invalid channel configuration being passed to the ADC");
  }

  if (HAL_ADC_Start(&adc_) != HAL_OK) {
    except<std::logic_error>("Could not start the ADC");
  }

  static constexpr uint32_t timeout_ms{10};
  if (HAL_ADC_PollForConversion(&adc_, timeout_ms) != HAL_OK) {
    except<std::runtime_error>(
        "HAL_ADC_PollForConversion failure. This is likely a catastrophic error.");
  }

  uint16_t result{static_cast<uint16_t>(HAL_ADC_GetValue(&adc_))};
  if (adc_index_ < adc_measured_values.size()) {
    adc_measured_values.at(adc_index_) = result;
  }
  HAL_ADC_Stop(&adc_);
  return result;
}

bool Stm32L4Adc::is_running() noexcept {
  const auto state{HAL_ADC_GetState(&adc_)};
  return (state & HAL_ADC_STATE_REG_BUSY) || (state & HAL_ADC_STATE_INJ_BUSY) ||
         (state & HAL_ADC_STATE_BUSY_INTERNAL);
}

void Stm32L4Adc::stop() noexcept {
  HAL_ADC_Stop_DMA(&adc_);
  dma_channel_.invalidate();
}

void initialize_for_calibration(ADC_HandleTypeDef& adc) noexcept {
  adc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  adc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  adc.Init.ScanConvMode = ADC_SCAN_DISABLE;
  adc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  adc.Init.LowPowerAutoWait = DISABLE;
  adc.Init.ContinuousConvMode = DISABLE;
  adc.Init.NbrOfConversion = 1;
  adc.Init.DiscontinuousConvMode = DISABLE;
  adc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  adc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  adc.Init.DMAContinuousRequests = DISABLE;
  adc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;

  HAL_ADC_Init(&adc);
}

void Stm32L4Adc::calibrate_single_ended_input() noexcept {
  stop();
  initialize_for_calibration(adc_);
  HAL_ADCEx_Calibration_Start(&adc_, ADC_SINGLE_ENDED);
}

void Stm32L4Adc::calibrate_differential_input() noexcept {
  stop();
  initialize_for_calibration(adc_);
  HAL_ADCEx_Calibration_Start(&adc_, ADC_DIFFERENTIAL_ENDED);
}

uint32_t Stm32L4Adc::read_calibration_factor() noexcept { return adc_.Instance->CALFACT; }

void Stm32L4Adc::write_calibration_factor(uint32_t factor) { adc_.Instance->CALFACT = factor; }

void Stm32L4Adc::enable() noexcept { __HAL_RCC_ADC_CLK_ENABLE(); }

void Stm32L4Adc::disable() noexcept {
  stop();
  __HAL_RCC_ADC_CLK_DISABLE();
}

}  // namespace tvsc::hal2::adc
