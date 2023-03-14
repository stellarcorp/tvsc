#include <RH_RF69.h>
#include <SPI.h>

#include <string>

#include "pb_decode.h"
#include "pb_encode.h"
#include "radio/radio_configuration.h"
#include "radio/rh_rf69_configuration.h"
#include "radio/settings.h"
#include "radio/settings.pb.h"
#include "random/random.h"

constexpr int RF69_RST{9};
constexpr int RF69_CS{10};

#if defined(RFM69_INTERRUPT_PIN)
constexpr int RF69_DIO0{RFM69_INTERRUPT_PIN};
#else
constexpr int RF69_DIO0{17};
#endif

RH_RF69 rf69{RF69_CS, digitalPinToInterrupt(RF69_DIO0)};

#ifdef TEENSY40
tvsc::radio::RadioConfiguration<RH_RF69> configuration{rf69, "RH_RF69 433 Teensy40"};
#elif TEENSY41
tvsc::radio::RadioConfiguration<RH_RF69> configuration{rf69, "RH_RF69 433 Teensy41"};
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

  pinMode(RF69_RST, OUTPUT);

  // Manual reset of board.
  // To reset, according to the datasheet, the reset pin needs to be high for 100us, then low for
  // 5ms, and then it will be ready. The pin should be pulled low by default on the radio module,
  // but we drive it low first anyway.
  digitalWrite(RF69_RST, LOW);
  delay(2);
  digitalWrite(RF69_RST, HIGH);
  delay(2);
  digitalWrite(RF69_RST, LOW);
  delay(7);

  if (!rf69.init()) {
    Serial.println("init failed");
    while (true) {
    }
  }

  Serial.print("Board id: ");
  print_id(configuration.identification());
  Serial.println();

  configuration.set_value(tvsc_radio_Function_CARRIER_FREQUENCY_HZ,
                          tvsc::radio::as_discrete_value(433e6f));

  configuration.set_value(tvsc_radio_Function_TX_POWER_DBM,
                          tvsc::radio::as_discrete_value<int8_t>(-2));

  configuration.set_value(tvsc_radio_Function_MODULATION_SCHEME,
                          tvsc::radio::as_discrete_value(tvsc_radio_ModulationTechnique_GFSK));

  configuration.set_value(
      tvsc_radio_Function_LINE_CODING,
      tvsc::radio::as_discrete_value(tvsc_radio_LineCoding_MANCHESTER_ORIGINAL));

  configuration.set_value(tvsc_radio_Function_BIT_RATE,
                          tvsc::radio::as_discrete_value<float>(70000.f));

  configuration.set_value(tvsc_radio_Function_FREQUENCY_DEVIATION,
                          tvsc::radio::as_discrete_value<float>(150000.f));

  configuration.commit_changes();
}

bool send(const std::string& msg) {
  bool result;
  result = rf69.send(msg.data(), msg.length());
  if (result) {
    result = rf69.waitPacketSent();
  }

  return result;
}

bool recv(std::string& buffer) {
  bool result;
  result = rf69.waitAvailableTimeout(1000);
  if (result) {
    buffer.clear();
    uint8_t length{rf69.maxMessageLength()};

    buffer.resize(length + 1);

    result = rf69.recv(buffer.data(), &length);
    if (result) {
      buffer.resize(length);
    }
  }

  return result;
}

template <typename MessageT>
void encode(const MessageT& message, std::string& buffer) {
  buffer.resize(rf69.maxMessageLength() + 1);
  pb_ostream_t ostream = pb_ostream_from_buffer(buffer.data(), rf69.maxMessageLength());
  bool status = pb_encode(&ostream, nanopb::MessageDescriptor<MessageT>::fields(), &message);
  if (!status) {
    tvsc::except<std::runtime_error>("Could not encode incoming message");
  }
  buffer.resize(ostream.bytes_written);
}

template <typename MessageT>
void decode(const std::string& buffer, MessageT& message) {
  pb_istream_t istream = pb_istream_from_buffer(buffer.data(), buffer.size());

  bool status = pb_decode(&istream, nanopb::MessageDescriptor<MessageT>::fields(), &message);
  if (!status) {
    tvsc::except<std::runtime_error>("Could not decode to outgoing message");
  }
}

void loop() {
  std::string id{};
  encode(configuration.identification(), id);

  std::string recv_buffer{};

  if (send(id)) {
    Serial.println("Published id.");
  } else {
    Serial.println("Send failed.");
  }

  delay(10);

  if (recv(recv_buffer)) {
    tvsc_radio_RadioIdentification other_id{};
    decode(recv_buffer, other_id);

    Serial.print("Received id: ");
    print_id(other_id);
    Serial.print("My id:");
    print_id(configuration.identification());
  }

  delay(tvsc::random::generate_random_value<uint8_t>(10, 255));
}
