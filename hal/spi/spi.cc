#include "bus/spi/spi.h"

namespace tvsc::bus::spi {

SpiTransaction::~SpiTransaction() {
  if (bus_ != nullptr) {
    bus_->end_transaction(peripheral_select_pin_);
  }
}

}  // namespace tvsc::bus::spi
