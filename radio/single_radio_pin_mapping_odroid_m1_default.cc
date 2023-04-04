#include "radio/single_radio_pin_mapping.h"

namespace tvsc::radio {

const char* SingleRadioPinMapping::board_name() { return "RF69HCW 433 Odroid M1"; }
int SingleRadioPinMapping::reset_pin() { return 106; }
int SingleRadioPinMapping::chip_select_pin() { return 14; }
int SingleRadioPinMapping::interrupt_pin() { return 12; }

}  // namespace tvsc::radio
