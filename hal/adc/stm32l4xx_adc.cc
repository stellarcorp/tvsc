#include "hal/adc/stm32l4xx_adc.h"

#include "hal/gpio/gpio.h"
#include "hal/peripheral_id.h"
#include "hal/stm32_peripheral_ids.h"

namespace tvsc::hal::adc {

static constexpr size_t NUMBER_RANKS_IN_SEQUENCER{16};

uint32_t trigger_flag(PeripheralId trigger_id) {
  if (trigger_id == Stm32PeripheralIds::TIM1_ID) {
    return ADC_EXTERNALTRIG_T1_TRGO;
  } else if (trigger_id == Stm32PeripheralIds::TIM2_ID) {
    return ADC_EXTERNALTRIG_T2_TRGO;
  } else if (trigger_id == Stm32PeripheralIds::TIM4_ID) {
    return ADC_EXTERNALTRIG_T4_TRGO;
  } else if (trigger_id == Stm32PeripheralIds::TIM6_ID) {
    return ADC_EXTERNALTRIG_T6_TRGO;
  } else if (trigger_id == Stm32PeripheralIds::TIM15_ID) {
    return ADC_EXTERNALTRIG_T15_TRGO;
  } else {
    return -1;
  }
}

static constexpr uint32_t get_channel(gpio::PortPin pin) {
  if (pin.port == 0) {
    if (pin.pin == 0) {
      // ADC1_IN5
      return ADC_CHANNEL_5;
    } else if (pin.pin == 1) {
      // ADC1_IN6
      return ADC_CHANNEL_6;
    } else if (pin.pin == 2) {
      // ADC1_IN7
      return ADC_CHANNEL_7;
    } else if (pin.pin == 3) {
      // ADC1_IN8
      return ADC_CHANNEL_8;
    } else if (pin.pin == 4) {
      // ADC1_IN9
      return ADC_CHANNEL_9;
    } else if (pin.pin == 5) {
      // ADC1_IN10
      return ADC_CHANNEL_10;
    } else if (pin.pin == 6) {
      // ADC1_IN11
      return ADC_CHANNEL_11;
    } else if (pin.pin == 7) {
      // ADC1_IN12
      return ADC_CHANNEL_12;
    }
  } else if (pin.port == 1) {
  } else if (pin.port == 2) {
  } else if (pin.port == 3) {
  } else if (pin.port == 4) {
  } else if (pin.port == 5) {
  } else if (pin.port == 6) {
  } else if (pin.port == 7) {
  }
  return 0xff;
}

void AdcStm32l4xx::start_conversion_stream(gpio::PortPin pin, uint32_t* destination,
                                           size_t destination_buffer_size, timer::Timer& trigger) {
  // Configure ADC.
  adc_.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  adc_.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  adc_.Init.ScanConvMode = ADC_SCAN_ENABLE;
  adc_.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  adc_.Init.LowPowerAutoWait = DISABLE;
  adc_.Init.ContinuousConvMode = DISABLE;
  adc_.Init.NbrOfConversion = 1;
  adc_.Init.DiscontinuousConvMode = DISABLE;
  adc_.Init.ExternalTrigConv = trigger_flag(trigger.id());
  adc_.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  adc_.Init.DMAContinuousRequests = ENABLE;

  HAL_ADC_Init(&adc_);

  // Configure an ADC Channel for each pin.
  channel_config_.Rank = 1;
  channel_config_.Channel = get_channel(pin);
  channel_config_.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
  channel_config_.SingleDiff = ADC_SINGLE_ENDED;
  channel_config_.Offset = 0;

  HAL_ADC_ConfigChannel(&adc_, &channel_config_);

  // Link DMA to ADC1. This allows the DMA's interrupt handler to (eventually) call the ADC's
  // interrupt handler, among other things.
  __HAL_LINKDMA(&adc_, DMA_Handle, *dma_->handle());

  dma_->start_circular_transfer();

  HAL_ADC_Start_DMA(&adc_, destination, destination_buffer_size);
}

void AdcStm32l4xx::start_single_conversion(gpio::PortPin pin, uint32_t* destination,
                                           size_t destination_buffer_size) {
  // Link DMA to ADC1. This allows the DMA's interrupt handler to (eventually) call the ADC's
  // interrupt handler, among other things.
  __HAL_LINKDMA(&adc_, DMA_Handle, *dma_->handle());

  // Configure ADC.
  adc_.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  adc_.Init.DataAlign = ADC_DATAALIGN_RIGHT;     // Right-aligned data
  adc_.Init.ScanConvMode = ADC_SCAN_DISABLE;     // Single channel
  adc_.Init.EOCSelection = ADC_EOC_SINGLE_CONV;  // End of conversion flag
  adc_.Init.LowPowerAutoWait = DISABLE;
  adc_.Init.ContinuousConvMode = DISABLE;  // Default single-shot
  adc_.Init.NbrOfConversion = 1;
  adc_.Init.DiscontinuousConvMode = DISABLE;
  adc_.Init.ExternalTrigConv = ADC_SOFTWARE_START;  // Start via software
  adc_.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;

  HAL_ADC_Init(&adc_);

  dma_->start_circular_transfer();

  // Configure ADC Channel.
  channel_config_.Channel = get_channel(pin);
  channel_config_.Rank = ADC_REGULAR_RANK_1;
  channel_config_.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
  channel_config_.SingleDiff = ADC_SINGLE_ENDED;
  channel_config_.Offset = 0;

  HAL_ADC_ConfigChannel(&adc_, &channel_config_);

  HAL_ADC_Start_DMA(&adc_, destination, destination_buffer_size);
}

void AdcStm32l4xx::reset_after_conversion() { stop(); }

void AdcStm32l4xx::set_resolution(uint8_t bits_resolution) {
  if (bits_resolution <= 6) {
    adc_.Init.Resolution = ADC_RESOLUTION_6B;
  } else if (bits_resolution <= 8) {
    adc_.Init.Resolution = ADC_RESOLUTION_8B;
  } else if (bits_resolution <= 10) {
    adc_.Init.Resolution = ADC_RESOLUTION_10B;
  } else if (bits_resolution <= 12) {
    adc_.Init.Resolution = ADC_RESOLUTION_12B;
  } else {
    adc_.Init.Resolution = ADC_RESOLUTION_12B;
  }
}

bool AdcStm32l4xx::is_running() {
  const auto state{HAL_ADC_GetState(&adc_)};
  return (state & HAL_ADC_STATE_REG_BUSY) || (state & HAL_ADC_STATE_INJ_BUSY) ||
         (state & HAL_ADC_STATE_BUSY_INTERNAL);
}

void AdcStm32l4xx::stop() { HAL_ADC_Stop_DMA(&adc_); }

void initialize_for_calibration(ADC_HandleTypeDef& adc) {
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

void AdcStm32l4xx::calibrate_single_ended_input() {
  stop();
  initialize_for_calibration(adc_);
  HAL_ADCEx_Calibration_Start(&adc_, ADC_SINGLE_ENDED);
}

void AdcStm32l4xx::calibrate_differential_input() {
  stop();
  initialize_for_calibration(adc_);
  HAL_ADCEx_Calibration_Start(&adc_, ADC_DIFFERENTIAL_ENDED);
}

uint32_t AdcStm32l4xx::read_calibration_factor() { return adc_.Instance->CALFACT; }

void AdcStm32l4xx::write_calibration_factor(uint32_t factor) { adc_.Instance->CALFACT = factor; }

void AdcStm32l4xx::handle_interrupt() { HAL_ADC_IRQHandler(&adc_); }

void AdcStm32l4xx::enable() { __HAL_RCC_ADC_CLK_ENABLE(); }
void AdcStm32l4xx::disable() { __HAL_RCC_ADC_CLK_DISABLE(); }

}  // namespace tvsc::hal::adc
