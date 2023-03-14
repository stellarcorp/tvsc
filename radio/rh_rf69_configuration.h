#pragma once

#include <ratio>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "RadioHead/RH_RF69.h"
#include "base/units.h"
#include "radio/radio.pb.h"
#include "radio/radio_configuration.h"

namespace tvsc::radio {

constexpr uint8_t SYNC_WORDS[] = "SR90tvsc";

/**
 * These templates define an interface for configuring the HopeRF RFM69HCW radio module using the
 * RadioHead RH_RF69 Driver.
 *
 * Manufacturer's documentation for the radio module:
 * https://www.hoperf.com/modules/rf_transceiver/RFM69HCW.html
 *
 * Note: this class covers the high-power variant tuned for ~433MHz.
 * TODO(james): When implementing the next variant, add template parameters for the tuning and the
 * power-level.
 *
 * From the RadioHead library documentation on this module:
 *   Packet Format
 *
 *   All messages sent and received by this RH_RF69 Driver conform to this packet format:
 *
 *   - 4 octets PREAMBLE
 *   - 2 octets SYNC 0x2d, 0xd4 (configurable, so you can use this as a network filter)
 *   - 1 octet RH_RF69 payload length
 *   - 4 octets HEADER: (TO, FROM, ID, FLAGS)
 *   - 0 to 60 octets DATA
 *   - 2 octets CRC computed with CRC16(IBM), computed on HEADER and DATA
 *
 * But that description leaves off these details:
 *
 * - The PREAMBLE can be configured to be from 0-65535 octets, though the comments indicate that at
 * least 2 octets are needed for stable communications. There are two 8-bit registers to store the
 * length of the preamble.
 *
 * - The length, as well as the contents, of the SYNC words are configurable. Allowed values for the
 * length are actually [0,8] where zero implies that the SYNC functionality should be turned off.
 * Not sure why, but the RadioHead implementation restricts the sync word size to 4 octets.
 *
 * - The implementation does not allow for an empty payload, so the effective range for the DATA
 * segment is 1-60 octets.
 *
 * - Longer payloads are possible if encryption is off. There might be some regulatory implications
 * though. Without encryption, we cannot send flight or control messages per FCC regulations. Maybe
 * our Tx/Rx API just includes a boolean indicating if the message is a control message and manages
 * that aspect of the configuration directly for those messages?
 */

template <>
std::unordered_map<tvsc_radio_Function, tvsc_radio_Value> generate_capabilities_map<RH_RF69>() {
  std::unordered_map<tvsc_radio_Function, tvsc_radio_Value> capabilities{};
  // We know the number of settings at compile time, and we know that number will not change.
  // Because of this, we can reserve the exact number of buckets we need and use a load factor of
  // 1 to save space. Also, since the number of elements is tiny, even if we "degrade" lookups to
  // O(n) with this setup, we will still see good performance.
  capabilities.max_load_factor(1.f);

  // Adafruit says we can go from 400MHz to 460MHz. Manufacturer says 424MHz to 510MHz. We measured
  // the range as 405MHz to 510MHz.
  capabilities.insert({tvsc_radio_Function_CARRIER_FREQUENCY_HZ,
                       float_range(in_unit<std::mega>(405), in_unit<std::mega>(510))});

  capabilities.insert(
      {tvsc_radio_Function_MODULATION_SCHEME,
       enumerated({tvsc_radio_ModulationTechnique_OOK, tvsc_radio_ModulationTechnique_FSK,
                   tvsc_radio_ModulationTechnique_MSK, tvsc_radio_ModulationTechnique_GFSK,
                   tvsc_radio_ModulationTechnique_GMSK})});

  capabilities.insert({tvsc_radio_Function_TX_POWER_DBM, int32_range(-2, 20)});

  // Note that it might be possible to go up to 65 bytes if we take
  // control of the header generation. The FIFOs have a size of 66 bytes,
  // but we would need to leave one byte for the message length.
  capabilities.insert({tvsc_radio_Function_MTU, int32_range(1, 60)});

  capabilities.insert({tvsc_radio_Function_PREAMBLE_LENGTH, int32_range(0, 0xffff)});

  capabilities.insert({tvsc_radio_Function_SYNC_WORDS_LENGTH, int32_range(0, 4)});

  capabilities.insert(
      {tvsc_radio_Function_LINE_CODING,
       enumerated({tvsc_radio_LineCoding_NONE, tvsc_radio_LineCoding_MANCHESTER_ORIGINAL,
                   tvsc_radio_LineCoding_WHITENING})});

  capabilities.insert(
      {tvsc_radio_Function_ENCRYPTION,
       enumerated({tvsc_radio_Encryption_NO_ENCRYPTION, tvsc_radio_Encryption_AES_128})});

  capabilities.insert(
      {tvsc_radio_Function_BIT_RATE, float_range(RH_RF69_FXOSC / 0xffff, RH_RF69_FXOSC)});

  capabilities.insert(
      {tvsc_radio_Function_FREQUENCY_DEVIATION, float_range(600.f, 0x3fff * RH_RF69_FSTEP)});

  return capabilities;
}

inline void get_frequency(RH_RF69& driver, tvsc_radio_DiscreteValue& value) {
  uint32_t step_count = driver.spiRead(RH_RF69_REG_07_FRFMSB);
  step_count = (step_count << 8) | driver.spiRead(RH_RF69_REG_08_FRFMID);
  step_count = (step_count << 8) | driver.spiRead(RH_RF69_REG_09_FRFLSB);
  const float frequency{step_count * RH_RF69_FSTEP};
  value.which_value = 2;
  value.value.float_value = frequency;
}

inline void get_preamble_length(RH_RF69& driver, tvsc_radio_DiscreteValue& value) {
  uint16_t length = driver.spiRead(RH_RF69_REG_2C_PREAMBLEMSB);
  length = (length << 8) | driver.spiRead(RH_RF69_REG_2D_PREAMBLELSB);
  value.which_value = 0;
  value.value.int32_value = length;
}

inline void get_sync_words_length(RH_RF69& driver, tvsc_radio_DiscreteValue& value) {
  uint8_t length = driver.spiRead(RH_RF69_REG_2E_SYNCCONFIG);
  length &= RH_RF69_SYNCCONFIG_SYNCSIZE;
  length >>= 3;
  length += 1;
  value.which_value = 0;
  value.value.int32_value = length;
}

inline void get_power(RH_RF69& driver, tvsc_radio_DiscreteValue& value) {
  // TODO(james): Expose the RH_RF69::_power data member to make this symmetric with the setter.
  int8_t power{-2};
  uint8_t palevel = driver.spiRead(RH_RF69_REG_11_PALEVEL);

  if ((palevel & RH_RF69_PALEVEL_PA2ON) && (palevel & RH_RF69_PALEVEL_PA1ON)) {
    // Note: +18dBm to +20dBm look just like +15dBm to +17dBm. The actual value is not exposed via
    // the RH_RF69 API.
    power = (palevel & RH_RF69_PALEVEL_OUTPUTPOWER) - 14;
  } else if (palevel & RH_RF69_PALEVEL_PA1ON) {
    power = (palevel & RH_RF69_PALEVEL_OUTPUTPOWER) - 18;
  }

  value.which_value = 0;
  value.value.int32_value = power;
}

inline void set_modulation_scheme(RH_RF69& driver, const tvsc_radio_DiscreteValue& value) {
  tvsc_radio_ModulationTechnique modulation{as<tvsc_radio_ModulationTechnique>(value)};
  uint8_t register_value{RH_RF69_DATAMODUL_DATAMODE_PACKET};
  switch (modulation) {
    case tvsc_radio_ModulationTechnique_OOK:
      register_value |=
          RH_RF69_DATAMODUL_MODULATIONTYPE_OOK | RH_RF69_DATAMODUL_MODULATIONSHAPING_OOK_NONE;
      break;
    case tvsc_radio_ModulationTechnique_FSK:
      register_value |=
          RH_RF69_DATAMODUL_MODULATIONTYPE_FSK | RH_RF69_DATAMODUL_MODULATIONSHAPING_FSK_NONE;
      break;
    case tvsc_radio_ModulationTechnique_GFSK:
      register_value |=
          RH_RF69_DATAMODUL_MODULATIONTYPE_FSK | RH_RF69_DATAMODUL_MODULATIONSHAPING_FSK_BT1_0;
      break;
    default:
      except<std::domain_error>("Invalid modulation technique");
  }
  driver.spiWrite(RH_RF69_REG_02_DATAMODUL, register_value);
}

inline void get_modulation_scheme(RH_RF69& driver, tvsc_radio_DiscreteValue& value) {
  uint8_t register_value = driver.spiRead(RH_RF69_REG_02_DATAMODUL);

  value.which_value = 0;
  if (register_value & RH_RF69_DATAMODUL_MODULATIONTYPE_FSK) {
    if (register_value & RH_RF69_DATAMODUL_MODULATIONSHAPING_FSK_BT1_0) {
      value.value.int32_value = tvsc_radio_ModulationTechnique_GFSK;
    } else {
      value.value.int32_value = tvsc_radio_ModulationTechnique_FSK;
    }
  } else if (register_value & RH_RF69_DATAMODUL_MODULATIONTYPE_OOK) {
    value.value.int32_value = tvsc_radio_ModulationTechnique_OOK;
  } else {
    except<std::domain_error>("Unknown modulation technique");
  }
}

inline void set_line_coding(RH_RF69& driver, const tvsc_radio_DiscreteValue& value) {
  tvsc_radio_LineCoding coding{as<tvsc_radio_LineCoding>(value)};
  uint8_t register_value{RH_RF69_PACKETCONFIG1_PACKETFORMAT_VARIABLE |
                         RH_RF69_PACKETCONFIG1_CRC_ON |
                         RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_NONE};
  switch (coding) {
    case tvsc_radio_LineCoding_NONE:
      register_value |= RH_RF69_PACKETCONFIG1_DCFREE_NONE;
      break;
    case tvsc_radio_LineCoding_WHITENING:
      register_value |= RH_RF69_PACKETCONFIG1_DCFREE_WHITENING;
      break;
    case tvsc_radio_LineCoding_MANCHESTER_ORIGINAL:
      register_value |= RH_RF69_PACKETCONFIG1_DCFREE_MANCHESTER;
      break;
    default:
      except<std::domain_error>("Invalid line coding");
  }
  driver.spiWrite(RH_RF69_REG_37_PACKETCONFIG1, register_value);
}

inline void get_line_coding(RH_RF69& driver, tvsc_radio_DiscreteValue& value) {
  uint8_t register_value = driver.spiRead(RH_RF69_REG_37_PACKETCONFIG1);

  value.which_value = 0;
  if (register_value & RH_RF69_PACKETCONFIG1_DCFREE_MANCHESTER) {
    value.value.int32_value = tvsc_radio_LineCoding_MANCHESTER_ORIGINAL;
  } else if (register_value & RH_RF69_PACKETCONFIG1_DCFREE_WHITENING) {
    value.value.int32_value = tvsc_radio_LineCoding_WHITENING;
  } else {
    value.value.int32_value = tvsc_radio_LineCoding_NONE;
  }
}

inline void set_bit_rate(RH_RF69& driver, const tvsc_radio_DiscreteValue& value) {
  float bit_rate{as<float>(value)};
  uint16_t bit_rate_register_values = RH_RF69_FXOSC / bit_rate;
  driver.spiWrite(RH_RF69_REG_03_BITRATEMSB, ((bit_rate_register_values >> 8) & 0xff));
  driver.spiWrite(RH_RF69_REG_04_BITRATELSB, bit_rate_register_values & 0xff);

  // Setting the bit rate also means that we need to set the Rx bandwidth. We set the Rx bandwidth
  // to be 2 * bit rate. Also, we put the same value in the AfcRxBw register.
  // See page 67 of the datasheet:
  // https://cdn-shop.adafruit.com/product-files/3076/RFM69HCW-V1.1.pdf for the register
  // descriptions. See page 26 for the lookup table driving the if-block that we use below.
  // TODO(james): Adjust lookup values after measurements.
  // TODO(james): Adjust lookup values for OOK modulation. Currently, these values are for FSK
  // variants. The datasheet recommends lowering the rx_bw thresholds by 2x for OOK.
  const float rx_bw = 2.f * bit_rate;
  uint8_t mantissa{};
  uint8_t exponent{};
  if (rx_bw < 2.6f) {
    mantissa = 0b10;
    exponent = 7;
  } else if (rx_bw < 3.1f) {
    mantissa = 0b01;
    exponent = 7;
  } else if (rx_bw < 3.9f) {
    mantissa = 0b00;
    exponent = 7;
  } else if (rx_bw < 5.2f) {
    mantissa = 0b10;
    exponent = 6;
  } else if (rx_bw < 6.3f) {
    mantissa = 0b01;
    exponent = 6;
  } else if (rx_bw < 7.8f) {
    mantissa = 0b00;
    exponent = 6;
  } else if (rx_bw < 10.4f) {
    mantissa = 0b10;
    exponent = 5;
  } else if (rx_bw < 12.5f) {
    mantissa = 0b01;
    exponent = 5;
  } else if (rx_bw < 15.6f) {
    mantissa = 0b00;
    exponent = 5;
  } else if (rx_bw < 20.8f) {
    mantissa = 0b10;
    exponent = 4;
  } else if (rx_bw < 25.0f) {
    mantissa = 0b01;
    exponent = 4;
  } else if (rx_bw < 31.3f) {
    mantissa = 0b00;
    exponent = 4;
  } else if (rx_bw < 41.7f) {
    mantissa = 0b10;
    exponent = 3;
  } else if (rx_bw < 50.0f) {
    mantissa = 0b01;
    exponent = 3;
  } else if (rx_bw < 62.5f) {
    mantissa = 0b00;
    exponent = 3;
  } else if (rx_bw < 83.3f) {
    mantissa = 0b10;
    exponent = 2;
  } else if (rx_bw < 100.0f) {
    mantissa = 0b01;
    exponent = 2;
  } else if (rx_bw < 125.0f) {
    mantissa = 0b00;
    exponent = 2;
  } else if (rx_bw < 166.7f) {
    mantissa = 0b10;
    exponent = 1;
  } else if (rx_bw < 200.0f) {
    mantissa = 0b01;
    exponent = 1;
  } else if (rx_bw < 250.0f) {
    mantissa = 0b00;
    exponent = 1;
  } else if (rx_bw < 333.3f) {
    mantissa = 0b10;
    exponent = 0;
  } else if (rx_bw < 400.0f) {
    mantissa = 0b01;
    exponent = 0;
  } else {  // rx_bw ~= 500.0f
    mantissa = 0b00;
    exponent = 0;
  }

  constexpr uint8_t rx_bw_dcc_freq{2};
  constexpr uint8_t afc_bw_dcc_freq{4};
  driver.spiWrite(RH_RF69_REG_19_RXBW, (rx_bw_dcc_freq << 5) | (mantissa << 3) | exponent);
  driver.spiWrite(RH_RF69_REG_1A_AFCBW, (afc_bw_dcc_freq << 5) | (mantissa << 3) | exponent);
}

inline void get_bit_rate(RH_RF69& driver, tvsc_radio_DiscreteValue& value) {
  uint8_t bit_rate_msb = driver.spiRead(RH_RF69_REG_03_BITRATEMSB);
  uint8_t bit_rate_lsb = driver.spiRead(RH_RF69_REG_04_BITRATELSB);

  value.which_value = 2;
  value.value.float_value = RH_RF69_FXOSC / ((bit_rate_msb << 8) | bit_rate_lsb);
}

inline void set_frequency_deviation(RH_RF69& driver, const tvsc_radio_DiscreteValue& value) {
  float deviation{as<float>(value)};
  uint16_t scaled_deviation{static_cast<uint16_t>(deviation / RH_RF69_FSTEP)};
  scaled_deviation &= 0x3fff;  // MSB two bits are unused.
  driver.spiWrite(RH_RF69_REG_05_FDEVMSB, ((scaled_deviation >> 8) & 0xff));
  driver.spiWrite(RH_RF69_REG_06_FDEVLSB, scaled_deviation & 0xff);
}

inline void get_frequency_deviation(RH_RF69& driver, tvsc_radio_DiscreteValue& value) {
  uint8_t deviation_msb = driver.spiRead(RH_RF69_REG_05_FDEVMSB);
  uint8_t deviation_lsb = driver.spiRead(RH_RF69_REG_06_FDEVLSB);

  value.which_value = 2;
  value.value.float_value = RH_RF69_FSTEP * ((deviation_msb << 8) | deviation_lsb);
}

template <>
tvsc_radio_DiscreteValue read_setting<RH_RF69>(RH_RF69& driver, tvsc_radio_Function function) {
  tvsc_radio_DiscreteValue value{};
  switch (function) {
    case tvsc_radio_Function_CARRIER_FREQUENCY_HZ: {
      get_frequency(driver, value);
      break;
    }
    case tvsc_radio_Function_PREAMBLE_LENGTH: {
      get_preamble_length(driver, value);
      break;
    }
    case tvsc_radio_Function_SYNC_WORDS_LENGTH: {
      get_sync_words_length(driver, value);
      break;
    }
    case tvsc_radio_Function_TX_POWER_DBM: {
      get_power(driver, value);
      break;
    }
    case tvsc_radio_Function_MODULATION_SCHEME: {
      get_modulation_scheme(driver, value);
      break;
    }
    case tvsc_radio_Function_LINE_CODING: {
      get_line_coding(driver, value);
      break;
    }
    case tvsc_radio_Function_BIT_RATE: {
      get_bit_rate(driver, value);
      break;
    }
    case tvsc_radio_Function_FREQUENCY_DEVIATION: {
      get_frequency_deviation(driver, value);
      break;
    }
  }
  return value;
}

template <>
void write_setting<RH_RF69>(RH_RF69& driver, tvsc_radio_Function function,
                            const tvsc_radio_DiscreteValue& value) {
  switch (function) {
    case tvsc_radio_Function_CARRIER_FREQUENCY_HZ: {
      float value_hz = as<float>(value);
      // RadioHead RH_RF69 code expects this value in MHz.
      driver.setFrequency(value_hz / 1'000'000);
      break;
    }
    case tvsc_radio_Function_PREAMBLE_LENGTH: {
      driver.setPreambleLength(as<uint16_t>(value));
      break;
    }
    case tvsc_radio_Function_SYNC_WORDS_LENGTH: {
      driver.setSyncWords(SYNC_WORDS, as<uint8_t>(value));
      break;
    }
    case tvsc_radio_Function_TX_POWER_DBM: {
      driver.setTxPower(as<int8_t>(value), /* ishighpowermodule */ true);
      break;
    }
    case tvsc_radio_Function_MODULATION_SCHEME: {
      set_modulation_scheme(driver, value);
      break;
    }
    case tvsc_radio_Function_LINE_CODING: {
      set_line_coding(driver, value);
      break;
    }
    case tvsc_radio_Function_BIT_RATE: {
      set_bit_rate(driver, value);
      break;
    }
    case tvsc_radio_Function_FREQUENCY_DEVIATION: {
      set_frequency_deviation(driver, value);
      break;
    }
  }
}

}  // namespace tvsc::radio
