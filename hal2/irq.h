#pragma once

#if defined(NUCLEO_BOARD)
#include "hal2/irq_arm.h"
#elif defined(SATELLITE)
#include "hal2/irq_arm.h"
#elif defined(GENERAL_PURPOSE_COMPUTER)
#include "hal2/irq_noop.h"
#else
#error \
    "Please configure this file to include the appropriate symbol (NUCLEO_L412KB, etc.) for the platform. Alternatively, update .bazelrc to define the appropriate symbol using --copt."
#endif
