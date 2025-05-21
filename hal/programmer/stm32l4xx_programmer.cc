#include "hal/programmer/stm32l4xx_programmer.h"

#include "hal/error.h"
#include "hal/gpio/gpio.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::programmer {

void ProgrammerStm32l4xx::enable() {
  using namespace tvsc::hal::gpio;

  gpio_ = gpio_peripheral_->access();
  gpio_.set_pin_mode(reset_pin_, PinMode::OUTPUT_PUSH_PULL_WITH_PULL_UP, PinSpeed::LOW);
}

void ProgrammerStm32l4xx::disable() { gpio_.invalidate(); }

void ProgrammerStm32l4xx::initiate_target_reset() { gpio_.write_pin(reset_pin_, 0); }
void ProgrammerStm32l4xx::conclude_target_reset() { gpio_.write_pin(reset_pin_, 1); }

void ProgrammerStm32l4xx::initiate_target_bootloader_boot() { gpio_.write_pin(boot0_pin_, 1); }
void ProgrammerStm32l4xx::conclude_target_bootloader_boot() { gpio_.write_pin(boot0_pin_, 0); }

}  // namespace tvsc::hal::programmer
