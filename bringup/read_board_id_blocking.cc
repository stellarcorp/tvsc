#include <chrono>
#include <cstdint>

#include "base/initializer.h"
#include "bringup/read_board_id.h"
#include "hal/board/board.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;
using namespace std::chrono_literals;
using namespace tvsc::hal::board_identification;

extern "C" {
alignas(uint32_t)  //
    __attribute__((section(".status.value"))) volatile BoardId board_id{};
}

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  BoardType& board{BoardType::board()};

  auto& gpio_id_power_peripheral{board.gpio<BoardType::BOARD_ID_POWER_PORT>()};
  auto& gpio_id_sense_peripheral{board.gpio<BoardType::BOARD_ID_SENSE_PORT>()};
  auto& adc_peripheral{board.adc()};

  while (true) {
    board_id = tvsc::bringup::read_board_id(gpio_id_power_peripheral, BoardType::BOARD_ID_POWER_PIN,
                                            gpio_id_sense_peripheral, BoardType::BOARD_ID_SENSE_PIN,
                                            adc_peripheral);

    ClockType::clock().wait(10ms);
  }
}
