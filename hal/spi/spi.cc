#include "hal/spi/spi.h"

namespace tvsc::hal::spi {

SpiTransaction::~SpiTransaction() {
  if (bus_ != nullptr) {
    bus_->end_transaction(peripheral_select_pin_);
  }
}

}  // namespace tvsc::hal::spi
