#include "comms/radio/single_radio_pin_mapping.h"

namespace tvsc::comms::radio {

const char* SingleRadioPinMapping::board_name() { return "RF69HCW 433MHz Teensy41"; }
uint8_t SingleRadioPinMapping::reset_pin() { return 9; }
uint8_t SingleRadioPinMapping::chip_select_pin() { return 10; }
uint8_t SingleRadioPinMapping::interrupt_pin() { return 24; }

}  // namespace tvsc::comms::radio
