#include "hal/board/board.h"

#include "flags/flags.h"

DEFINE_uint64(run_duration_sec, 24 * 60 * 60, "How long to run this script in seconds");

namespace tvsc::hal::board {

// One board per executable.
Board Board::board_{};

}  // namespace tvsc::hal::board
