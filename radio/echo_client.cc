#include <Entropy.h>

#include <string>

#include "hal/gpio/pins.h"
#include "hal/output/output.h"
#include "hal/spi/spi.h"
#include "hal/time/time.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "radio/packet.pb.h"
#include "radio/radio_configuration.h"
#include "radio/rf69hcw_configuration.h"
#include "radio/settings.h"
#include "radio/settings.pb.h"
#include "radio/single_radio_pin_mapping.h"
#include "random/random.h"

const uint8_t RF69_RST{tvsc::radio::SingleRadioPinMapping::reset_pin()};
const uint8_t RF69_CS{tvsc::radio::SingleRadioPinMapping::chip_select_pin()};
const uint8_t RF69_DIO0{tvsc::radio::SingleRadioPinMapping::interrupt_pin()};

tvsc::hal::spi::SpiBus bus{tvsc::hal::spi::get_default_spi_bus()};
tvsc::hal::spi::SpiPeripheral spi_peripheral{bus, RF69_CS, 0x80};
tvsc::radio::RF69HCW rf69{};

tvsc::radio::RadioConfiguration<tvsc::radio::RF69HCW> configuration{
    rf69, tvsc::radio::SingleRadioPinMapping::board_name()};

void print_id(const tvsc_radio_RadioIdentification& id) {
  tvsc::hal::output::print("{");
  tvsc::hal::output::print(id.expanded_id);
  tvsc::hal::output::print(", ");
  tvsc::hal::output::print(id.id);
  tvsc::hal::output::print(", ");
  tvsc::hal::output::print(id.name);
  tvsc::hal::output::println("}");
}

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

void setup() {
  Serial.begin(9600);

  Entropy.Initialize();
  tvsc::random::set_seed(Entropy.random());
  configuration.regenerate_identifiers();

  tvsc::hal::gpio::set_mode(RF69_RST, tvsc::hal::gpio::PinMode::MODE_OUTPUT);

  // Manual reset of board.
  // To reset, according to the datasheet, the reset pin needs to be high for 100us, then low for
  // 5ms, and then it will be ready. The pin should be pulled low by default on the radio module,
  // but we drive it low first anyway.
  tvsc::hal::gpio::write_pin(RF69_RST, tvsc::hal::gpio::DigitalValue::VALUE_LOW);
  tvsc::hal::time::delay_ms(10);
  tvsc::hal::gpio::write_pin(RF69_RST, tvsc::hal::gpio::DigitalValue::VALUE_HIGH);
  tvsc::hal::time::delay_ms(10);
  tvsc::hal::gpio::write_pin(RF69_RST, tvsc::hal::gpio::DigitalValue::VALUE_LOW);
  tvsc::hal::time::delay_ms(10);

  bus.init();

  if (!rf69.init(spi_peripheral, RF69_DIO0)) {
    tvsc::hal::output::println("init failed");
    while (true) {
    }
  }

  tvsc::hal::output::print("Board id: ");
  print_id(configuration.identification());
  tvsc::hal::output::println();

  configuration.change_values(tvsc::radio::default_configuration<tvsc::radio::RF69HCW>());
  configuration.commit_changes();
}

bool send(const std::string& msg) {
  bool result;
  result = rf69.send(reinterpret_cast<const uint8_t*>(msg.data()), msg.length());
  if (result) {
    result = rf69.wait_packet_sent();
  }

  return result;
}

void encode_packet(uint32_t protocol, uint32_t sequence_number, uint32_t id,
                   const std::string& message, std::string& buffer) {
  tvsc_radio_Packet packet{};
  packet.protocol = protocol;
  packet.sequence_number = sequence_number;
  packet.sender = id;
  packet.payload.size = std::min(message.length(), 62U);
  strncpy(packet.payload.bytes, message.data(), packet.payload.size);

  buffer.resize(rf69.mtu());
  pb_ostream_t ostream =
      pb_ostream_from_buffer(reinterpret_cast<uint8_t*>(buffer.data()), buffer.capacity());
  bool status =
      pb_encode(&ostream, nanopb::MessageDescriptor<tvsc_radio_Packet>::fields(), &packet);
  if (!status) {
    tvsc::except<std::runtime_error>("Could not encode packet for message");
  }
  buffer.resize(ostream.bytes_written);
}

uint32_t sequence_number{};
void loop() {
  std::string packet{};
  packet.resize(rf69.mtu());

  encode_packet(1, ++sequence_number, configuration.id(), "Hello, world!", packet);

  if (send(packet)) {
    tvsc::hal::output::println("Sent.");
  } else {
    tvsc::hal::output::print("send() failed. RSSI: ");
    tvsc::hal::output::println(rf69.read_rssi_dbm());

    // Resend the "same" packet.
    --sequence_number;
  }

  delay(250);
}
