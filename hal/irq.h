#pragma once

#pragma once

#if defined(NUCLEO_H743ZI)
#include "hal/irq_arm.h"
#elif defined(NUCLEO_L412KB)
#include "hal/irq_arm.h"
#elif defined(NUCLEO_L432KC)
#include "hal/irq_arm.h"
#elif defined(NUCLEO_L452RE)
#include "hal/irq_arm.h"
#elif defined(GENERAL_PURPOSE_COMPUTER)
#include "hal/irq_noop.h"
#else
#error \
    "Please configure this file to include the appropriate symbol (NUCLEO_L412KB, etc.) for the platform. Alternatively, update .bazelrc to define the appropriate symbol using --copt."
#endif
