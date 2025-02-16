#include "hal/dac/dac.h"

namespace tvsc::hal::dac {

Dac DacPeripheral::access() { return Dac(*this); }

}  // namespace tvsc::hal::dac
