#include "hal/board/simulation_board.h"

#include "glog/logging.h"

namespace tvsc::hal::board {

Board& Board::board() {
  static bool first_time{true};
  if (first_time) {
    LOG(INFO) << "Simulation log file: " << board_.logger_.log_file_name();
    first_time = false;
  }
  return board_;
}

Board::Board() {}

}  // namespace tvsc::hal::board
