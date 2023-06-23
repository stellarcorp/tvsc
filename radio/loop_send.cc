#include <string>

#include "hal/gpio/pins.h"
#include "hal/output/output.h"
#include "hal/spi/spi.h"
#include "hal/time/time.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "radio/nanopb_proto/packet.pb.h"
#include "radio/nanopb_proto/settings.pb.h"
#include "radio/radio_configuration.h"
#include "radio/rf69hcw_configuration.h"
#include "radio/settings.h"
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

  configuration.change_values(tvsc::radio::high_throughput_configuration());
  configuration.commit_changes();

  uint32_t sequence_number{};

  std::string identification_message{};
  {
    identification_message.resize(rf69.mtu());
    pb_ostream_t ostream =
        pb_ostream_from_buffer(reinterpret_cast<uint8_t*>(identification_message.data()),
                               identification_message.capacity());
    bool status = pb_encode(
        &ostream, nanopb::MessageDescriptor<tvsc_radio_nano_RadioIdentification>::fields(),
        &configuration.identification());
    if (!status) {
      tvsc::except<std::runtime_error>("Could not encode message");
    }
    identification_message.resize(ostream.bytes_written);
  }

  while (true) {
    tvsc::radio::Fragment<tvsc::radio::RF69HCW::max_mtu()> fragment{};
    tvsc::radio::encode_packet(1, ++sequence_number, configuration.id(), identification_message,
                               fragment);

    if (tvsc::radio::send(rf69, fragment)) {
      tvsc::hal::output::println("Published id.");
    } else {
      tvsc::hal::output::print("send() failed. RSSI: ");
      tvsc::hal::output::println(rf69.read_rssi_dbm());
    }

    tvsc::hal::time::delay_us(2425);
  }

  return 0;
}
