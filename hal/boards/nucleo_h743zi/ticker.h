#pragma once

#include <cstdint>
#include <new>

#include "hal/register.h"
#include "hal/time/clock.h"

extern "C" {

#include "stm32h7xx.h"

/**
 * Number of ticks before the SysTick interrupt is fired. Effectively, this is ticks per interrupt.
 */
extern uint32_t tick_frequency;

/**
 * Current time in microseconds where "current time" means time since last reset.
 */
extern volatile CTimeType current_time_us;

/**
 * Interrupt handler for SysTick. Handles scenario when the SysTick counter
 * (SysTickRegisterBank::VAL) reaches zero and reloads its value from SysTickRegisterBank::LOAD.
 */
void SysTick_Handler();
}

namespace tvsc::hal::boards::nucleo_h743zi {

class SysTickRegisterBank final {
 public:
  //
  volatile Register CTRL;
  volatile Register LOAD;
  volatile Register VAL;
  volatile Register CALIB;
};

/**
 * Class to manage the reset and clock circuitry (RCC) on the Nucleo H743ZI board.
 */
class Ticker final {
 private:
  SysTickRegisterBank* registers_;

 public:
  Ticker(void* base_address) : registers_(new (base_address) SysTickRegisterBank) {
    tick_frequency = SystemCoreClock / 10000;
    SysTick_Config(tick_frequency);
  }
};

}  // namespace tvsc::hal::boards::nucleo_h743zi
