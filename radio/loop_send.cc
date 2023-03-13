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

void setup() {
  Serial.begin(9600);

  pinMode(RF69_RST, OUTPUT);
  digitalWrite(RF69_RST, LOW);
  delay(10);

  Serial.println("Teensy RFM69 Client!");
  Serial.println();

  // Manual reset of board.
  digitalWrite(RF69_RST, HIGH);
  delay(10);
  digitalWrite(RF69_RST, LOW);
  delay(10);

  if (!rf69.init()) {
    Serial.println("init failed");
    while (true) {
    }
  }

  // Use a very slow encoding/modulation scheme like this so that the signal stands out.
  // rf69.setModemConfig(RH_RF69::OOK_Rb1Bw1);
  // Use a faster scheme.
  // rf69.setModemConfig(RH_RF69::OOK_Rb32Bw64);
  // Fast/high bandwidth GFSK modulation scheme.
  // rf69.setModemConfig(RH_RF69::GFSK_Rb250Fd250);
  // Fast/high bandwidth FSK modulation scheme.
  // rf69.setModemConfig(RH_RF69::FSK_Rb250Fd250);
  // Slow FSK modulation scheme.
  rf69.setModemConfig(RH_RF69::FSK_Rb2_4Fd4_8);
  // rf69.setModemConfig(RH_RF69::GFSK_Rb2_4Fd4_8);
  // rf69.setModemConfig(RH_RF69::FSK_Rb57_6Fd120);

  configuration.set_value(tvsc_radio_Function_CARRIER_FREQUENCY_HZ,
                          tvsc::radio::as_discrete_value(433e6f));
  configuration.set_value(tvsc_radio_Function_TX_POWER_DBM,
                          tvsc::radio::as_discrete_value<int8_t>(-2));
  configuration.set_value(tvsc_radio_Function_MODULATION_SCHEME,
                          tvsc::radio::as_discrete_value(tvsc_radio_ModulationTechnique_GFSK));
  configuration.set_value(tvsc_radio_Function_LINE_CODING,
                          tvsc::radio::as_discrete_value(tvsc_radio_LineCoding_NONE));

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
