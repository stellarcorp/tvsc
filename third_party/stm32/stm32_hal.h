#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if defined(STM32H743xx)
#include "stm32h7xx_hal.h"
#elif defined(STM32L412xx)
#include "stm32l4xx_hal.h"
#elif defined(STM32L432xx)
#include "stm32l4xx_hal.h"
#elif defined(STM32L452xx)
#include "stm32l4xx_hal.h"
#else
#error \
    "Please configure this file to include the appropriate symbol (STM32H743xx, STM32L452xx, etc.) for the platform. Alternatively, update .bazelrc to define the appropriate symbol using --copt."
#endif

#ifdef __cplusplus
}
#endif
