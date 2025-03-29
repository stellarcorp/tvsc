#include <array>
#include <cstdint>

#include "base/initializer.h"
#include "hal/board/board.h"
#include "hal/i2c/i2c.h"

using BoardType = tvsc::hal::board::Board;
using namespace tvsc::hal::i2c;

extern "C" {

__attribute__((section(".status.value"))) std::array<uint8_t, NUM_VALID_I2C_ADDRESSES>
    discovered_devices{};

}  // extern "C"

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  BoardType& board{BoardType::board()};
  auto& i2c_peripheral{board.i2c1()};

  auto i2c{i2c_peripheral.access()};
  while (true) {
    tvsc::hal::i2c::scan_bus(i2c, discovered_devices);
  }
}
