#pragma once

#if defined(STM32L412xx)
#include "hal/mcu/stm32l4xx.h"
#elif defined(STM32L432xx)
#include "hal/mcu/stm32l4xx.h"
#elif defined(STM32L452xx)
#include "hal/mcu/stm32l4xx.h"
#else
#error \
    "Please configure this file to include the appropriate MCU symbol (STM32L412xx, etc.) for the platform. Alternatively, update .bazelrc to define the appropriate symbol using --copt."
#endif
