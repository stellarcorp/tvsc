#include "bus/spi/spi.h"

namespace tvsc::bus::spi {

SpiTransaction::~SpiTransaction() {
  if (bus_ != nullptr) {
    bus_->end_transaction();
  }
}

SpiPeripheralSelection::~SpiPeripheralSelection() {
  if (bus_ != nullptr) {
    bus_->deselect_peripheral(peripheral_select_pin_);
  }
}

}  // namespace tvsc::bus::spi
