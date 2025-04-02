#include <array>
#include <cstdint>

#include "base/initializer.h"
#include "hal/board/board.h"
#include "hal/i2c/i2c.h"

using BoardType = tvsc::hal::board::Board;

struct alignas(16) ImuIds final {
  uint16_t imu1_id{};
  uint16_t imu2_id{};
  bool imu1_read_success{};
  bool imu2_read_success{};
};

extern "C" {

__attribute__((section(".status.value"))) ImuIds ids{};

}  // extern "C"

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  BoardType& board{BoardType::board()};
  auto& imu1_peripheral{board.imu1()};
  auto& imu2_peripheral{board.imu2()};

  auto imu1{imu1_peripheral.access()};
  auto imu2{imu2_peripheral.access()};

  while (true) {
    ids.imu2_read_success = imu2.read_id(ids.imu2_id);
    ids.imu1_read_success = imu1.read_id(ids.imu1_id);
  }
}
