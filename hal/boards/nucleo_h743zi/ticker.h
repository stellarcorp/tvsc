#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <new>

#include "hal/boards/register.h"
#include "hal/gpio/pins.h"
#include "hal/time/time.h"

typedef tvsc::hal::time::TimeType CTimeType;

extern "C" {

#include "stm32h7xx.h"

/**
 * Number of ticks before the SysTick interrupt is fired. Effectively, this is ticks per interrupt.
 */
extern uint32_t tick_frequency;

/**
 * Current time in microseconds where "current time" means time since last reset.
 */
extern CTimeType current_time_us;

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
    // Generate a tick interrupt every 1000 ticks, regardless of clock speed. We don't want to
    // change the interrupt rate every time we change the clock frequency. This means that the
    // precision of our time is based on the clock speed.
    tick_frequency = 1000;
    SysTick_Config(tick_frequency);
  }
};

}  // namespace tvsc::hal::boards::nucleo_h743zi
