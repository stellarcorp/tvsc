#pragma once

#if defined(CHEERFUL_COFFEE)
#include "hal2/board/cheerful_coffee_board.h"
#else
#error \
    "Please configure this file to include the appropriate symbol (NUCLEO_L412KB, etc.) for the platform. Alternatively, update .bazelrc to define the appropriate symbol using --copt."
#endif
