#pragma once

#include <cstdint>
#include <string>

namespace tvsc::comms::radio {

enum class ModulationScheme {
  UNINITIALIZED = 0,

  // Digital data.

  // Amplitude-based techniques.

  // https://en.wikipedia.org/wiki/On%E2%80%93off_keying
  OOK = 1,

  // https://en.wikipedia.org/wiki/Amplitude-shift_keying
  ASK = 2,

  // https://en.wikipedia.org/wiki/Amplitude_and_phase-shift_keying
  APSK = 3,

  // Frequency-based techniques.
  // https://en.wikipedia.org/wiki/Frequency-shift_keying
  FSK = 4,

  // https://en.wikipedia.org/wiki/Frequency-shift_keying
  AFSK = 5,

  // https://en.wikipedia.org/wiki/Frequency-shift_keying and
  // https://en.wikipedia.org/wiki/Project_25
  C4FM = 6,

  // https://en.wikipedia.org/wiki/Frequency-shift_keying#Gaussian_frequency-shift_keying
  GFSK = 7,

  // https://en.wikipedia.org/wiki/Multiple_frequency-shift_keying
  MFSK = 8,

  // https://en.wikipedia.org/wiki/Minimum-shift_keying
  MSK = 9,

  // https://en.wikipedia.org/wiki/Minimum-shift_keying#Gaussian_minimum-shift_keying
  GMSK = 10,

  // Phase-based techniques.

  // https://en.wikipedia.org/wiki/Pulse-position_modulation
  PPM = 11,

  // https://en.wikipedia.org/wiki/Continuous_phase_modulation
  CPM = 12,

  // https://en.wikipedia.org/wiki/Phase-shift_keying
  PSK = 13,

  // https://en.wikipedia.org/wiki/Phase-shift_keying#Quadrature_phase-shift_keying_.28QPSK.29
  QPSK = 14,

  // https://en.wikipedia.org/wiki/Phase-shift_keying#Offset_QPSK_(OQPSK)
  OQPSK = 15,

  // https://en.wikipedia.org/wiki/Quadrature_amplitude_modulation
  QAM = 16,
  // https://en.wikipedia.org/wiki/Single-carrier_FDMA
  SC_FDMA = 17,

  // https://en.wikipedia.org/wiki/Trellis_modulation (* ILMP research)
  TCM = 18,

  // https://en.wikipedia.org/wiki/Wavelet_modulation (* ILMP research)
  WDM = 19,

  // Spread spectrum techniques. https://en.wikipedia.org/wiki/Spread_spectrum
  // https://en.wikipedia.org/wiki/Chirp_spread_spectrum (* ILMP research)
  CSS = 20,

  // https://en.wikipedia.org/wiki/Direct-sequence_spread_spectrum (* ILMP research)
  DSSS = 21,

  // https://en.wikipedia.org/wiki/Frequency-hopping_spread_spectrum (* ILMP research)
  FHSS = 22,

  // https://en.wikipedia.org/wiki/Time-hopping
  THSS = 23,

  // Analog data.
  AM = 24,
  FM = 25,
  PM = 26,

  // https://en.wikipedia.org/wiki/Single-sideband_modulation
  USB = 27,

  // https://en.wikipedia.org/wiki/Single-sideband_modulation
  LSB = 28,
};

enum class LineCoding {
  NONE = 0,

  WHITENING = 1,

  // This list primarily comes from:
  // https://en.wikipedia.org/wiki/Line_code#Transmission_and_storage
  NRZ_L = 2,
  NRZ_M = 3,
  NRZ_S = 4,
  RZ = 5,
  BIPHASE_L = 6,
  BIPHASE_M = 7,
  BIPHASE_S = 8,

  // Manchester encoding and differential Manchester encoding both have multiple implementations.
  MANCHESTER_ORIGINAL = 9,
  MANCHESTER_802_3 = 10,

  DIFFERENTIAL_MANCHESTER = 11,

  BIPOLAR = 12,
};

enum class Encryption {
  NO_ENCRYPTION = 0,
  AES_128 = 1,
};

}  // namespace tvsc::comms::radio
