#include <RH_RF69.h>
#include <SPI.h>

#include <string>

#include "radio/radio_configuration.h"
#include "radio/rh_rf69_configuration.h"
#include "radio/settings.h"
#include "radio/settings.pb.h"

constexpr int RF69_RST{9};
constexpr int RF69_CS{10};

#if defined(RFM69_INTERRUPT_PIN)
constexpr int RF69_DIO0{RFM69_INTERRUPT_PIN};
#else
constexpr int RF69_DIO0{17};
#endif

RH_RF69 rf69{RF69_CS, digitalPinToInterrupt(RF69_DIO0)};
tvsc::radio::RadioConfiguration<RH_RF69> configuration{rf69, "RH_RF69"};

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

  Serial.println("Teensy RFM69 Client!");
  Serial.println();

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
                          tvsc::radio::as_discrete_value<float>(57600.f));

  configuration.set_value(tvsc_radio_Function_FREQUENCY_DEVIATION,
                          tvsc::radio::as_discrete_value<float>(120000.f));

  configuration.commit_changes();
}

bool send(const std::string& msg) {
  Serial.println("Sending to rf69_server");

  bool result;
  result = rf69.send(msg.data(), msg.length());
  Serial.println("Send requested.");

  if (result) {
    result = rf69.waitPacketSent();
  }

  if (result) {
    Serial.println("Sent.");
  }

  return result;
}

bool recv(std::string& buffer) {
  bool result;
  result = rf69.waitAvailableTimeout(10);
  if (result) {
    buffer.clear();
    uint8_t length{rf69.maxMessageLength()};

    buffer.resize(length + 1);

    result = rf69.recv(buffer.data(), &length);
    if (result) {
      buffer.resize(length);
      Serial.print("got reply: ");
      Serial.println(buffer.c_str());
    } else {
      Serial.println("recv failed");
    }
  } else {
    Serial.println("No reply available.");
  }

  return result;
}

void loop() {
  constexpr char data[] = "Hello World!";
  std::string buffer{};

  send(data);

  if (recv(buffer)) {
    Serial.print("Received: ");
    Serial.println(buffer.c_str());
  }
}
