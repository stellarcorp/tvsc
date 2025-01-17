#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if defined(STM32H743xx)
#include "stm32h743xx.h"
#elif defined(STM32L452xx)
#include "stm32l452xx.h"
#else
#error \
    "Please configure toolchain to define the appropriate symbol (STM32H743xx, STM32L452xx, etc.) for the platform"
#endif

#ifdef __cplusplus
}
#endif
