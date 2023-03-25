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
#include "random/random.h"

constexpr uint8_t RF69_RST{9};
constexpr uint8_t RF69_CS{10};

#if defined(RFM69_INTERRUPT_PIN)
constexpr uint8_t RF69_DIO0{RFM69_INTERRUPT_PIN};
#else
constexpr uint8_t RF69_DIO0{17};
#endif

SPISettings spi_settings{};
tvsc::radio::RF69HCW rf69{RF69_CS, digitalPinToInterrupt(RF69_DIO0), SPI, spi_settings};

#ifdef TEENSY40
tvsc::radio::RadioConfiguration<tvsc::radio::RF69HCW> configuration{rf69, "RF69HCW 433 Teensy40"};
#elif TEENSY41
tvsc::radio::RadioConfiguration<tvsc::radio::RF69HCW> configuration{rf69, "RF69HCW 433 Teensy41"};
#endif

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

  configuration.change_values(tvsc::radio::high_throughput_configuration());
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

template <typename MessageT>
void encode_packet(uint32_t protocol, uint32_t sequence_number, uint32_t id,
                   const MessageT& message, std::string& buffer) {
  {
    buffer.resize(rf69.mtu());
    pb_ostream_t ostream =
        pb_ostream_from_buffer(reinterpret_cast<uint8_t*>(buffer.data()), buffer.capacity());
    bool status = pb_encode(&ostream, nanopb::MessageDescriptor<MessageT>::fields(), &message);
    if (!status) {
      tvsc::except<std::runtime_error>("Could not encode message");
    }
    buffer.resize(ostream.bytes_written);
  }

  tvsc_radio_Packet packet{};
  packet.protocol = protocol;
  packet.sequence_number = sequence_number;
  packet.sender = id;
  packet.payload.size = buffer.size();
  strncpy(packet.payload.bytes, buffer.data(), buffer.size());

  {
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
}

uint32_t sequence_number{};
void loop() {
  std::string packet{};
  encode_packet(1, ++sequence_number, configuration.id(), configuration.identification(), packet);

  if (send(packet)) {
    Serial.println("Published id.");
  } else {
    Serial.print("send() failed. RSSI: ");
    Serial.println(rf69.read_rssi_dbm());
  }

  delayMicroseconds(2425);
}
