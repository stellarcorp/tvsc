#include "hal/timer/stm32l4xx_timer.h"

#include "hal/enable_lock.h"
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
  timer_.Init.Period = 10'000; //interval_us / clock_divider - 1;
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

void disable(PeripheralId id) {
  if (id == Stm32PeripheralIds::TIM1_ID) {
    __HAL_RCC_TIM1_CLK_DISABLE();
  } else if (id == Stm32PeripheralIds::TIM2_ID) {
    __HAL_RCC_TIM2_CLK_DISABLE();
  } else if (id == Stm32PeripheralIds::TIM3_ID) {
    __HAL_RCC_TIM3_CLK_DISABLE();
  } else if (id == Stm32PeripheralIds::TIM4_ID) {
    __HAL_RCC_TIM2_CLK_DISABLE();
  } else if (id == Stm32PeripheralIds::TIM6_ID) {
    __HAL_RCC_TIM6_CLK_DISABLE();
  } else if (id == Stm32PeripheralIds::TIM15_ID) {
    __HAL_RCC_TIM15_CLK_DISABLE();
  }
}

EnableLock TimerStm32l4xx::enable() {
  if (enable_counter_ == 0) {
    if (id_ == Stm32PeripheralIds::TIM1_ID) {
      __HAL_RCC_TIM1_CLK_ENABLE();
    } else if (id_ == Stm32PeripheralIds::TIM2_ID) {
      __HAL_RCC_TIM2_CLK_ENABLE();
    } else if (id_ == Stm32PeripheralIds::TIM3_ID) {
      __HAL_RCC_TIM3_CLK_ENABLE();
    } else if (id_ == Stm32PeripheralIds::TIM4_ID) {
      __HAL_RCC_TIM2_CLK_ENABLE();
    } else if (id_ == Stm32PeripheralIds::TIM6_ID) {
      __HAL_RCC_TIM6_CLK_ENABLE();
    } else if (id_ == Stm32PeripheralIds::TIM15_ID) {
      __HAL_RCC_TIM15_CLK_ENABLE();
    }
  }
  ++enable_counter_;
  return EnableLock([this]() {
    --enable_counter_;
    if (enable_counter_ == 0) {
      disable(id_);
    }
  });
}

void TimerStm32l4xx::handle_interrupt() { HAL_TIM_IRQHandler(&timer_); }

}  // namespace tvsc::hal::timer
