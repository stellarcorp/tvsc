#include "hal/timer/stm32l4xx_timer.h"

#include "hal/peripheral_id.h"
#include "hal/stm32_peripheral_ids.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::timer {

PeripheralId TimerStm32l4xx::id() { return id_; }

void TimerStm32l4xx::start(uint32_t interval_us, bool high_precision) {
  const uint32_t clock_divider{high_precision ? 1U : 4U};
  const uint32_t clock_division{high_precision ? TIM_CLOCKDIVISION_DIV1 : TIM_CLOCKDIVISION_DIV4};

  // Configure TIMER.
  timer_.Init.Prescaler = SystemCoreClock / 1'000'000 - 1;  // One tick per us.
  timer_.Init.CounterMode = TIM_COUNTERMODE_UP;
  timer_.Init.Period = 10'000;  // interval_us / clock_divider - 1;
  timer_.Init.ClockDivision = clock_division;
  timer_.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  HAL_TIM_Base_Init(&timer_);

  TIM_MasterConfigTypeDef master_config{};
  master_config.MasterOutputTrigger = TIM_TRGO_UPDATE;
  master_config.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&timer_, &master_config);

  // Start timer.
  HAL_TIM_Base_Start(&timer_);
}

bool TimerStm32l4xx::is_running() { return HAL_TIM_Base_GetState(&timer_) == HAL_TIM_STATE_BUSY; }

void TimerStm32l4xx::stop() { HAL_TIM_Base_Stop(&timer_); }

void TimerStm32l4xx::disable() {
  if (id_ == Stm32PeripheralIds::TIM1_ID) {
    __HAL_RCC_TIM1_CLK_DISABLE();
  } else if (id_ == Stm32PeripheralIds::TIM2_ID) {
    __HAL_RCC_TIM2_CLK_DISABLE();
  } else if (id_ == Stm32PeripheralIds::TIM4_ID) {
    __HAL_RCC_TIM2_CLK_DISABLE();
  } else if (id_ == Stm32PeripheralIds::TIM6_ID) {
    __HAL_RCC_TIM6_CLK_DISABLE();
  } else if (id_ == Stm32PeripheralIds::TIM15_ID) {
    __HAL_RCC_TIM15_CLK_DISABLE();
  }
}

void TimerStm32l4xx::enable() {
  if (id_ == Stm32PeripheralIds::TIM1_ID) {
    __HAL_RCC_TIM1_CLK_ENABLE();
  } else if (id_ == Stm32PeripheralIds::TIM2_ID) {
    __HAL_RCC_TIM2_CLK_ENABLE();
  } else if (id_ == Stm32PeripheralIds::TIM4_ID) {
    __HAL_RCC_TIM2_CLK_ENABLE();
  } else if (id_ == Stm32PeripheralIds::TIM6_ID) {
    __HAL_RCC_TIM6_CLK_ENABLE();
  } else if (id_ == Stm32PeripheralIds::TIM15_ID) {
    __HAL_RCC_TIM15_CLK_ENABLE();
  }
}

void TimerStm32l4xx::handle_interrupt() { HAL_TIM_IRQHandler(&timer_); }

PeripheralId Stm32l4xxLptim::id() { return id_; }

void Stm32l4xxLptim::start(uint32_t interval_us, bool /*high_precision*/) {
  // This next line assigns the clock source to be the LSI. I think. Need to determine how the
  // line in enable() and the init structure in start() interact.
  timer_.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  timer_.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  timer_.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  timer_.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  timer_.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;

  static constexpr uint32_t LSI_FREQ_HZ{32000};

  uint32_t timeout_ticks{LSI_FREQ_HZ / 1'000 * interval_us / 1'000};
  if (interval_us < 2048000) {
    timer_.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
  } else if (interval_us < 4'096'000) {
    timer_.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV2;
    timeout_ticks /= 2;
  } else if (interval_us < 8'192'000) {
    timer_.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV4;
    timeout_ticks /= 4;
  } else if (interval_us < 16'384'000) {
    timer_.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV8;
    timeout_ticks /= 8;
  } else if (interval_us < 32'768'000) {
    timer_.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV16;
    timeout_ticks /= 16;
  } else if (interval_us < 65'536'000) {
    timer_.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32;
    timeout_ticks /= 32;
  } else if (interval_us < 131'072'000) {
    timer_.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV64;
    timeout_ticks /= 64;
  } else {
    timer_.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV128;
    timeout_ticks /= 128;
  }

  HAL_LPTIM_Init(&timer_);

  is_running_ = true;

  HAL_LPTIM_TimeOut_Start_IT(&timer_, 0xFFFF, timeout_ticks);
}

bool Stm32l4xxLptim::is_running() { return is_running_; }

void Stm32l4xxLptim::stop() {
  is_running_ = false;
  HAL_LPTIM_TimeOut_Stop_IT(&timer_);
}

void Stm32l4xxLptim::disable() {
  if (id_ == Stm32PeripheralIds::LPTIM1_ID) {
    __HAL_RCC_LPTIM1_CLK_DISABLE();
  } else if (id_ == Stm32PeripheralIds::LPTIM2_ID) {
    __HAL_RCC_LPTIM2_CLK_DISABLE();
  }

  lsi_active_.invalidate();
}

void Stm32l4xxLptim::enable() {
  lsi_active_ = oscillator_->access();

  if (id_ == Stm32PeripheralIds::LPTIM1_ID) {
    // This next line assigns the clock source to be the LSI. I think. Need to determine how the
    // line in enable() and the init structure in start() interact.
    __HAL_RCC_LPTIM1_CONFIG(RCC_LPTIM1CLKSOURCE_LSI);
    __HAL_RCC_LPTIM1_CLK_ENABLE();
  } else if (id_ == Stm32PeripheralIds::LPTIM2_ID) {
    __HAL_RCC_LPTIM2_CONFIG(RCC_LPTIM2CLKSOURCE_LSI);
    __HAL_RCC_LPTIM2_CLK_ENABLE();
  }
}

void Stm32l4xxLptim::handle_interrupt() {
  is_running_ = false;
  HAL_LPTIM_IRQHandler(&timer_);
}

}  // namespace tvsc::hal::timer
