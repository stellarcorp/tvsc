#pragma once

#if defined(NUCLEO_H743ZI)
#include "hal/pinout/nucleo_h743zi_pinout.h"
#elif defined(NUCLEO_L412KB)
#include "hal/pinout/nucleo_l412kb_pinout.h"
#elif defined(NUCLEO_L432KC)
#include "hal/pinout/nucleo_l432kc_pinout.h"
#elif defined(NUCLEO_L452RE)
#include "hal/pinout/nucleo_l452re_pinout.h"
#elif defined(FEATURE_BRINGUP)
#include "hal/pinout/feature_bringup_pinout.h"
#elif defined(SATELLITE)
#include "hal/pinout/satellite_pinout.h"
#elif defined(GENERAL_PURPOSE_COMPUTER)
#include "hal/pinout/simulation_pinout.h"
#else
#error \
    "Please configure this file to include the appropriate symbol (NUCLEO_L412KB, etc.) for the platform. Alternatively, update .bazelrc to define the appropriate symbol using --copt."
#endif
