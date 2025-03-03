#include <string>

#include "comms/radio/fragment.h"
#include "comms/radio/radio_utilities.h"
#include "comms/radio/rf69hcw.h"
#include "comms/radio/settings.h"
#include "comms/radio/single_radio_pin_mapping.h"
#include "comms/radio/transceiver_identification.h"
#include "hal/gpio/pins.h"
#include "hal/output/output.h"
#include "hal/spi/spi.h"
#include "time/time.h"
#include "random/random.h"

/**
 * TODO(james): For telemetry, monitor the following:
 *
 * - power
 * - rssi (ambient)
 * - rssi (receiving)
 * - snr: the ratio of the receiving rssi to the ambient rssi.
 * - modulation index: 2 * Fdev / BR
 * - afc frequency error
 * - temperature of radio module
 */

using namespace tvsc::comms::radio;
using RadioT = RF69HCW;
using FragmentT = Fragment<RadioT::max_mtu()>;

const uint8_t RF69_RST{SingleRadioPinMapping::reset_pin()};
const uint8_t RF69_CS{SingleRadioPinMapping::chip_select_pin()};
const uint8_t RF69_DIO0{SingleRadioPinMapping::interrupt_pin()};

int main() {
  tvsc::random::initialize_seed();
  tvsc::hal::gpio::initialize_gpio();

  tvsc::hal::spi::SpiBus bus{tvsc::hal::spi::get_default_spi_bus()};
  tvsc::hal::spi::SpiPeripheral spi_peripheral{bus, RF69_CS, 0x80};
  RadioT rf69{spi_peripheral, RF69_DIO0, RF69_RST};
  TransceiverIdentification identification{TransceiverIdentification::initialize()};

  tvsc::hal::output::print("Board id: ");
  tvsc::hal::output::println(to_string(identification));

  uint32_t sequence_number{};

  while (true) {
    static constexpr char MESSAGE[] = "Hello, world!";
    FragmentT fragment{};
    fragment.set_sequence_number(++sequence_number);
    // fragment.set_sender_id(configuration.id());
    strncpy(reinterpret_cast<char*>(fragment.payload_start()), MESSAGE,
            FragmentT::max_payload_size());
    fragment.set_payload_size(sizeof(MESSAGE));

    if (send(rf69, fragment)) {
      tvsc::hal::output::println("Sent.");
      tvsc::hal::output::print("Board id: ");
      tvsc::hal::output::println(to_string(identification));

    } else {
      tvsc::hal::output::println("send() failed.");

      // Resend the same fragment by reverting the sequence_number increment above.
      --sequence_number;
    }

    rf69.set_standby_mode();
    tvsc::time::delay_ms(1000);
  }

  return 0;
}
