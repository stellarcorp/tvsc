#pragma once

#if defined(STM32H743xx)
#include "hal/board/nucleo_h743zi_board.h"
#elif defined(STM32L412xx)
#include "hal/board/nucleo_l412kb_board.h"
#elif defined(STM32L432xx)
#include "hal/board/nucleo_l432kc_board.h"
#elif defined(STM32L452xx)
#include "hal/board/nucleo_l452re_board.h"
#else
#error \
    "Please configure this file to include the appropriate symbol (STM32H743xx, STM32L452xx, etc.) for the platform. Alternatively, update .bazelrc to define the appropriate symbol using --copt."
#endif
