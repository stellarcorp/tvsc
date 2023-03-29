#include <Arduino.h>
#include <Entropy.h>
#include <SPI.h>

#include <string>

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

SPISettings spi_settings{};
tvsc::radio::RF69HCW rf69{RF69_CS, digitalPinToInterrupt(RF69_DIO0), SPI, spi_settings};

tvsc::radio::RadioConfiguration<tvsc::radio::RF69HCW> configuration{
    rf69, tvsc::radio::SingleRadioPinMapping::board_name()};

void print_id(const tvsc_radio_RadioIdentification& id) {
  Serial.print("{");
  Serial.print(id.expanded_id);
  Serial.print(", ");
  Serial.print(id.id);
  Serial.print(", ");
  Serial.print(id.name);
  Serial.println("}");
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

  pinMode(RF69_RST, OUTPUT);

  // Manual reset of board.
  // To reset, according to the datasheet, the reset pin needs to be high for 100us, then low for
  // 5ms, and then it will be ready. The pin should be pulled low by default on the radio module,
  // but we drive it low first anyway.
  digitalWrite(RF69_RST, LOW);
  delay(10);
  digitalWrite(RF69_RST, HIGH);
  delay(10);
  digitalWrite(RF69_RST, LOW);
  delay(10);

  if (!rf69.init()) {
    Serial.println("init failed");
    while (true) {
    }
  }

  Serial.print("Board id: ");
  print_id(configuration.identification());
  Serial.println();

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
    Serial.println("Sent.");
  } else {
    Serial.print("send() failed. RSSI: ");
    Serial.println(rf69.read_rssi_dbm());

    // Resend the "same" packet.
    --sequence_number;
  }

  delay(250);
}
