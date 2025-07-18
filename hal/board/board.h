#pragma once

#include "flags/flags.h"

DECLARE_uint64(run_duration_sec);

#if defined(NUCLEO_H743ZI)
#include "hal/board/nucleo_h743zi_board.h"
#elif defined(NUCLEO_L412KB)
#include "hal/board/nucleo_l412kb_board.h"
#elif defined(NUCLEO_L432KC)
#include "hal/board/nucleo_l432kc_board.h"
#elif defined(NUCLEO_L452RE)
#include "hal/board/nucleo_l452re_board.h"
#elif defined(FEATURE_BRINGUP)
#include "hal/board/feature_bringup_board.h"
#elif defined(SATELLITE)
#include "hal/board/satellite_board.h"
#elif defined(GENERAL_PURPOSE_COMPUTER)
#include "hal/board/simulation_board.h"
#else
#error \
    "Please configure this file to include the appropriate symbol (NUCLEO_L412KB, etc.) for the platform. Alternatively, update .bazelrc to define the appropriate symbol using --copt."
#endif
