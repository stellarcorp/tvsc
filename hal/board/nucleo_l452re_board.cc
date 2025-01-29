#include "hal/board/nucleo_l452re_board.h"

#include "third_party/stm32/stm32.h"

namespace tvsc::hal::board {

GPIO_TypeDef* Board::GREEN_LED_PORT_HAL{GPIOA};
uint32_t Board::GREEN_LED_PIN_HAL{1U << 5};

}  // namespace tvsc::hal::board
