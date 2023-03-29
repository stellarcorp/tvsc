#pragma once

namespace tvsc::radio {

class SingleRadioPinMapping final {
 public:
  static const char* board_name();

  /**
   * Pin connected to the RF69HCW reset line.
   */
  static int reset_pin();

  /**
   * Pin connected to the SPI Chip Select line on the RF69HCW. Note that the RF69HCW datasheet
   * designation for this line is NSS.
   */
  static int chip_select_pin();

  /**
   * Pin connected to the GPIO0 interrupt line of the Adafruit breakout board for the RF69HCW.
   *
   * Note that there are actually 6 interrupt lines on the RF69HCW. Hope RF, the chip manufacturer,
   * refers to these as DIO0 through DIO5. Adafruit, the breakout board manufacturer, refers these
   * lines as GPIO0 through GPIO5, with the markings on the board showing G0 through G5. So, when
   * looking at the board wiring, this pin refers to the pin connected to the daughterboard's G0
   * pin.
   */
  static int interrupt_pin();
};

}  // namespace tvsc::radio
