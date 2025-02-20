#include "hal/board/board.h"
#include "hal/time/clock.h"

namespace tvsc::hal::time {

Clock& system_clock() { return board::Board::board().clock(); }

}  // namespace tvsc::hal::time
