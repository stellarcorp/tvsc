#include "hal/board/satellite_board.h"

#include "hal/mcu/mcu.h"

namespace tvsc::hal::board {

mcu::Mcu& Board::mcu() { return mcu::Mcu::mcu(); }
Board& Board::board() { return board_; }

}  // namespace tvsc::hal::board
