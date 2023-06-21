#include <string>

#include "hal/gpio/pins.h"
#include "hal/output/output.h"
#include "hal/spi/spi.h"
#include "hal/time/time.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "radio/encoding.h"
#include "radio/fragment.h"
#include "radio/packet.h"
#include "radio/radio_configuration.h"
#include "radio/rf69hcw.h"
#include "radio/rf69hcw_configuration.h"
#include "radio/settings.h"
#include "radio/settings.pb.h"
#include "radio/single_radio_pin_mapping.h"
#include "radio/utilities.h"
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

const uint8_t RF69_RST{tvsc::radio::SingleRadioPinMapping::reset_pin()};
const uint8_t RF69_CS{tvsc::radio::SingleRadioPinMapping::chip_select_pin()};
const uint8_t RF69_DIO0{tvsc::radio::SingleRadioPinMapping::interrupt_pin()};

int main() {
  tvsc::random::initialize_seed();
  tvsc::hal::gpio::initialize_gpio();

  tvsc::hal::spi::SpiBus bus{tvsc::hal::spi::get_default_spi_bus()};
  tvsc::hal::spi::SpiPeripheral spi_peripheral{bus, RF69_CS, 0x80};
  tvsc::radio::RF69HCW rf69{spi_peripheral, RF69_DIO0, RF69_RST};

  tvsc::radio::RadioConfiguration<tvsc::radio::RF69HCW> configuration{
      rf69, tvsc::radio::SingleRadioPinMapping::board_name()};

  tvsc::hal::output::print("Board id: ");
  tvsc::radio::print_id(configuration.identification());
  tvsc::hal::output::println();

  configuration.change_values(tvsc::radio::default_configuration<tvsc::radio::RF69HCW>());
  configuration.commit_changes();

  uint32_t sequence_number{};

  while (true) {
    static constexpr unsigned char MESSAGE[] = "Hello, world!";
    tvsc::radio::Packet packet{};
    packet.set_protocol(tvsc::radio::Protocol::TVSC_CONTROL);
    packet.set_sequence_number(++sequence_number);
    packet.set_sender_id(configuration.id());
    packet.payload().write_array(0, sizeof(MESSAGE), MESSAGE);
    packet.set_payload_length(sizeof(MESSAGE));
    tvsc::radio::EncodedPacket<tvsc::radio::RF69HCW::max_mtu(), 1> fragment{};

    tvsc::radio::encode(packet, fragment);

    if (tvsc::radio::send(rf69, fragment.buffers[0])) {
      tvsc::hal::output::println("Sent.");
      tvsc::hal::output::print("Board id: ");
      tvsc::radio::print_id(configuration.identification());
      tvsc::hal::output::println();

    } else {
      tvsc::hal::output::println("send() failed.");

      // Resend the "same" packet.
      --sequence_number;
    }

    rf69.set_standby_mode();
    tvsc::hal::time::delay_ms(1000);
  }

  return 0;
}
