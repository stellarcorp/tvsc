#include "radio/single_radio_pin_mapping.h"

namespace tvsc::radio {

const char* SingleRadioPinMapping::board_name() { return "RF69HCW 433MHz Odroid M1"; }
uint8_t SingleRadioPinMapping::reset_pin() { return 106; }
uint8_t SingleRadioPinMapping::chip_select_pin() { return 14; }
uint8_t SingleRadioPinMapping::interrupt_pin() { return 12; }

}  // namespace tvsc::radio
