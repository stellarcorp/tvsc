#include <string>

#include "comms/radio/fragment.h"
#include "comms/radio/nanopb_proto/settings.pb.h"
#include "comms/radio/radio_configuration.h"
#include "comms/radio/radio_utilities.h"
#include "comms/radio/rf69hcw.h"
#include "comms/radio/rf69hcw_configuration.h"
#include "comms/radio/settings.h"
#include "comms/radio/single_radio_pin_mapping.h"
#include "hal/gpio/pins.h"
#include "hal/output/output.h"
#include "hal/spi/spi.h"
#include "hal/time/time.h"
#include "pb_decode.h"
#include "pb_encode.h"
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

using RadioT = tvsc::comms::radio::RF69HCW;
using FragmentT = tvsc::comms::radio::Fragment<RadioT::max_mtu()>;

const uint8_t RF69_RST{tvsc::comms::radio::SingleRadioPinMapping::reset_pin()};
const uint8_t RF69_CS{tvsc::comms::radio::SingleRadioPinMapping::chip_select_pin()};
const uint8_t RF69_DIO0{tvsc::comms::radio::SingleRadioPinMapping::interrupt_pin()};

int main() {
  tvsc::random::initialize_seed();
  tvsc::hal::gpio::initialize_gpio();

  tvsc::hal::spi::SpiBus bus{tvsc::hal::spi::get_default_spi_bus()};
  tvsc::hal::spi::SpiPeripheral spi_peripheral{bus, RF69_CS, 0x80};
  RadioT rf69{spi_peripheral, RF69_DIO0, RF69_RST};

  tvsc::comms::radio::RadioConfiguration<RadioT> configuration{
      rf69, tvsc::comms::radio::SingleRadioPinMapping::board_name()};

  tvsc::hal::output::print("Board id: ");
  tvsc::comms::radio::print_id(configuration.identification());
  tvsc::hal::output::println();

  configuration.change_values(tvsc::comms::radio::default_configuration<RadioT>());
  configuration.commit_changes();

  uint32_t sequence_number{};

  while (true) {
    static constexpr char MESSAGE[] = "Hello, world!";
    FragmentT fragment{};
    fragment.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
    fragment.set_sequence_number(++sequence_number);
    fragment.set_sender_id(configuration.id());
    strncpy(reinterpret_cast<char*>(fragment.payload_start()), MESSAGE, sizeof(MESSAGE));
    fragment.set_payload_size(sizeof(MESSAGE));

    if (tvsc::comms::radio::send(rf69, fragment)) {
      tvsc::hal::output::println("Sent.");
      tvsc::hal::output::print("Board id: ");
      tvsc::comms::radio::print_id(configuration.identification());
      tvsc::hal::output::println();

    } else {
      tvsc::hal::output::println("send() failed.");

      // Resend the "same" fragment.
      --sequence_number;
    }

    rf69.set_standby_mode();
    tvsc::hal::time::delay_ms(1000);
  }

  return 0;
}
