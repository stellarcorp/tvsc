#include "hal/gpio/stm_gpio.h"

#include <new>

#include "hal/gpio/gpio.h"
#include "hal/register.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::gpio {

void GpioStm32xxxx::set_ospeedr_value(Pin pin, PinSpeed speed) {
  uint8_t speed_value{};
  switch (speed) {
    case PinSpeed::LOW:
      speed_value = 0b00;
      break;
    case PinSpeed::MEDIUM:
      speed_value = 0b01;
      break;
    case PinSpeed::HIGH:
      speed_value = 0b10;
      break;
    case PinSpeed::VERY_HIGH:
      speed_value = 0b11;
      break;
      // default:
      // except<std::domain_error>("Requested pin speed is not supported on this platform.");
  }
  registers_->OSPEEDR.set_bit_field_value<2>(speed_value, static_cast<uint8_t>(2 * pin));
}

void GpioStm32xxxx::set_moder_value(Pin pin, MODER_VALUES value) {
  registers_->MODER.set_bit_field_value<2>(static_cast<uint32_t>(value),
                                           static_cast<uint8_t>(2 * pin));
}

void GpioStm32xxxx::set_pupdr_value(Pin pin, OPUPDR_VALUES value) {
  registers_->OPUPDR.set_bit_field_value<2>(static_cast<uint32_t>(value),
                                            static_cast<uint8_t>(2 * pin));
}

void GpioStm32xxxx::set_otyper_value(Pin pin, OTYPER_VALUES value) {
  registers_->OTYPER.set_bit_field_value<1>(static_cast<uint32_t>(value),
                                            static_cast<uint8_t>(pin));
}

void GpioStm32xxxx::set_alternate_function(Pin pin, uint8_t value) {
  if (pin >= 0 && pin < 8) {
    registers_->AFRL.set_bit_field_value<4>(value, 4 * pin);
  } else {
    registers_->AFRH.set_bit_field_value<4>(value, 4 * (pin - 8));
  }
}

void GpioStm32xxxx::set_pin_mode(Pin pin, PinMode mode, PinSpeed speed,
                                 uint8_t alternate_function_mapping) {
  switch (mode) {
      // Unused pins get set to the reset state of most pins. Note that the reset state of some
      // pins, especially debug pins, is different.
    case PinMode::UNUSED:
      set_moder_value(pin, MODER_VALUES::ANALOG);
      set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
      set_ospeedr_value(pin, speed);
      set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
      break;

      // Analog mode needs the mode configured, and the pull-up/pull-down resistors must be
      // disabled. Speed and type are ignored.
    case PinMode::ANALOG:
      set_moder_value(pin, MODER_VALUES::ANALOG);
      set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
      break;

      // Input modes need the mode and pull-up/pull-down configured. Speed and type are ignored.
    case PinMode::INPUT_FLOATING:
      set_moder_value(pin, MODER_VALUES::INPUT);
      set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
      break;

    case PinMode::INPUT_WITH_PULL_UP:
      set_moder_value(pin, MODER_VALUES::INPUT);
      set_pupdr_value(pin, OPUPDR_VALUES::PULL_UP_ENABLED);
      break;

    case PinMode::INPUT_WITH_PULL_DOWN:
      set_moder_value(pin, MODER_VALUES::INPUT);
      set_pupdr_value(pin, OPUPDR_VALUES::PULL_DOWN_ENABLED);
      break;

      // Output modes need the mode, output type, output speed, and pull-up/pull-down configured.
    case PinMode::OUTPUT_PUSH_PULL:
      set_moder_value(pin, MODER_VALUES::OUTPUT);
      set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
      set_ospeedr_value(pin, speed);
      set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
      break;

    case PinMode::OUTPUT_PUSH_PULL_WITH_PULL_UP:
      set_moder_value(pin, MODER_VALUES::OUTPUT);
      set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
      set_ospeedr_value(pin, speed);
      set_pupdr_value(pin, OPUPDR_VALUES::PULL_UP_ENABLED);
      break;

    case PinMode::OUTPUT_PUSH_PULL_WITH_PULL_DOWN:
      set_moder_value(pin, MODER_VALUES::OUTPUT);
      set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
      set_ospeedr_value(pin, speed);
      set_pupdr_value(pin, OPUPDR_VALUES::PULL_DOWN_ENABLED);
      break;

    case PinMode::OUTPUT_OPEN_DRAIN:
      set_moder_value(pin, MODER_VALUES::OUTPUT);
      set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
      set_ospeedr_value(pin, speed);
      set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
      break;

    case PinMode::OUTPUT_OPEN_DRAIN_WITH_PULL_UP:
      set_moder_value(pin, MODER_VALUES::OUTPUT);
      set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
      set_ospeedr_value(pin, speed);
      set_pupdr_value(pin, OPUPDR_VALUES::PULL_UP_ENABLED);
      break;

    case PinMode::OUTPUT_OPEN_DRAIN_WITH_PULL_DOWN:
      set_moder_value(pin, MODER_VALUES::OUTPUT);
      set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
      set_ospeedr_value(pin, speed);
      set_pupdr_value(pin, OPUPDR_VALUES::PULL_DOWN_ENABLED);
      break;

      // Alternate function modes need the mode, type, speed, and pull-up/pull-down configured.
    case PinMode::ALTERNATE_FUNCTION_PUSH_PULL:
      set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
      set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
      set_ospeedr_value(pin, speed);
      set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
      set_alternate_function(pin, alternate_function_mapping);
      break;

    case PinMode::ALTERNATE_FUNCTION_PUSH_PULL_WITH_PULL_UP:
      set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
      set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
      set_ospeedr_value(pin, speed);
      set_pupdr_value(pin, OPUPDR_VALUES::PULL_UP_ENABLED);
      set_alternate_function(pin, alternate_function_mapping);
      break;

    case PinMode::ALTERNATE_FUNCTION_PUSH_PULL_WITH_PULL_DOWN:
      set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
      set_otyper_value(pin, OTYPER_VALUES::MOSFET_PUSH_PULL);
      set_ospeedr_value(pin, speed);
      set_pupdr_value(pin, OPUPDR_VALUES::PULL_DOWN_ENABLED);
      set_alternate_function(pin, alternate_function_mapping);
      break;

    case PinMode::ALTERNATE_FUNCTION_OPEN_DRAIN:
      set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
      set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
      set_ospeedr_value(pin, speed);
      set_pupdr_value(pin, OPUPDR_VALUES::BOTH_DISABLED);
      set_alternate_function(pin, alternate_function_mapping);
      break;

    case PinMode::ALTERNATE_FUNCTION_OPEN_DRAIN_WITH_PULL_UP:
      set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
      set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
      set_ospeedr_value(pin, speed);
      set_pupdr_value(pin, OPUPDR_VALUES::PULL_UP_ENABLED);
      set_alternate_function(pin, alternate_function_mapping);
      break;

    case PinMode::ALTERNATE_FUNCTION_OPEN_DRAIN_WITH_PULL_DOWN:
      set_moder_value(pin, MODER_VALUES::ALTERNATE_FUNCTION);
      set_otyper_value(pin, OTYPER_VALUES::MOSFET_OPEN_DRAIN);
      set_ospeedr_value(pin, speed);
      set_pupdr_value(pin, OPUPDR_VALUES::PULL_DOWN_ENABLED);
      set_alternate_function(pin, alternate_function_mapping);
      break;

      // default:
      // except<std::domain_error>("Requested pin mode is not supported on this platform.");
  }
}

void GpioStm32xxxx::write_pin(Pin pin, bool on) {
  // Atomically turn on or off the pin. Setting a one in the lower 16-bits turns ON the pin.
  // Setting a one in the upper 16-bits turns OFF the pin.
  if (on) {
    registers_->BSRR.set_value(1U << pin);
  } else {
    registers_->BSRR.set_value((1U << pin) << NUM_PINS);
  }
}

void GpioStm32xxxx::toggle_pin(Pin pin) {
  const uint32_t current_value{registers_->ODR.value()};
  registers_->BSRR.set_value(((current_value & (1U << pin)) << NUM_PINS) |
                             (~current_value & (1U << pin)));
}

bool GpioStm32xxxx::read_pin(Pin pin) { return registers_->IDR.bit_field_value<1>(pin); }

void GpioStm32xxxx::disable() {
  if (port_ == 0) {
    __HAL_RCC_GPIOA_CLK_DISABLE();
  } else if (port_ == 1) {
    __HAL_RCC_GPIOB_CLK_DISABLE();
  } else if (port_ == 2) {
    __HAL_RCC_GPIOC_CLK_DISABLE();
  } else if (port_ == 3) {
#if defined(__HAL_RCC_GPIOD_CLK_DISABLE)
    __HAL_RCC_GPIOD_CLK_DISABLE();
#endif
  } else if (port_ == 4) {
#if defined(__HAL_RCC_GPIOE_CLK_DISABLE)
    __HAL_RCC_GPIOE_CLK_DISABLE();
#endif
  } else if (port_ == 7) {
    __HAL_RCC_GPIOH_CLK_DISABLE();
  }
}

void GpioStm32xxxx::enable() {
  if (port_ == 0) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
  } else if (port_ == 1) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
  } else if (port_ == 2) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
  } else if (port_ == 3) {
#if defined(__HAL_RCC_GPIOD_CLK_ENABLE)
    __HAL_RCC_GPIOD_CLK_ENABLE();
#endif
  } else if (port_ == 4) {
#if defined(__HAL_RCC_GPIOE_CLK_ENABLE)
    __HAL_RCC_GPIOE_CLK_ENABLE();
#endif
  } else if (port_ == 7) {
    __HAL_RCC_GPIOH_CLK_ENABLE();
  }
}

}  // namespace tvsc::hal::gpio
