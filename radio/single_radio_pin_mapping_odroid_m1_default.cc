#include "radio/single_radio_pin_mapping.h"

namespace tvsc::radio {

const char* SingleRadioPinMapping::board_name() { return "RF69HCW 433 Odroid M1"; }
int SingleRadioPinMapping::reset_pin() { return 9; }
int SingleRadioPinMapping::chip_select_pin() { return 10; }
int SingleRadioPinMapping::interrupt_pin() { return 17; }

}  // namespace tvsc::radio