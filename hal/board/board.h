#pragma once

#include "flags/flags.h"

DECLARE_uint64(run_duration_sec);

#if defined(NUCLEO_BOARD)
#include "hal/board/nucleo_board.h"
#elif defined(SATELLITE)
#include "hal/board/satellite_board.h"
#elif defined(GENERAL_PURPOSE_COMPUTER)
#include "hal/board/simulation_board.h"
#else
#error \
    "Please configure this file to include the appropriate symbol (NUCLEO_L412KB, etc.) for the platform. Alternatively, update .bazelrc to define the appropriate symbol using --copt."
#endif
