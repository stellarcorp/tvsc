#include "hal/gpio/gpio.h"

namespace tvsc::hal::gpio {

Gpio GpioPeripheral::access() { return Gpio(*this); }

}  // namespace tvsc::hal::gpio
