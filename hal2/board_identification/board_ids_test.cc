#include "hal2/board_identification/board_ids.h"

#include <limits>

#include "base/enums.h"
#include "gtest/gtest.h"

namespace tvsc::hal2::board_identification {

TEST(BoardIdTest, NoIdZero) {
  for (const auto& configuration : board_id_voltage_dividers()) {
    EXPECT_NE(0, configuration.id());
  }
}

TEST(BoardIdTest, NoIdAtUnknownBoardId) {
  for (const auto& configuration : board_id_voltage_dividers()) {
    EXPECT_NE(UNKNOWN_BOARD_ID, configuration.id());
  }
}

TEST(BoardIdTest, NoIdAtMaxValue) {
  static constexpr BoardId MAX_VALUE{std::numeric_limits<BoardId>::max()};
  for (const auto& configuration : board_id_voltage_dividers()) {
    EXPECT_NE(MAX_VALUE, configuration.id());
  }
}

TEST(BoardIdTest, ExpectedNumberOfConfigurations) {
  // Test if the requested parameters (NUM_BOARD_IDS, BOARD_ID_ADC_RESOLUTION_BITS,
  // BOARD_ID_RESISTOR_TOLERANCE, VOLTAGE_DIVIDER_TOTAL_RESISTANCE, etc.) can give an expected set
  // of configurations.
  EXPECT_EQ(NUM_BOARD_IDS, board_id_voltage_dividers().size());
}

TEST(BoardIdTest, NoOverlapsInAdcRanges) {
  const auto& configurations{board_id_voltage_dividers()};
  for (size_t i = 0; i < configurations.size() - 1; ++i) {
    const uint16_t min_adc_measurement{configurations[i].min_adc_measurement_value()};
    const uint16_t max_adc_measurement{configurations[i].max_adc_measurement_value()};
    for (size_t j = i + 1; j < configurations.size(); ++j) {
      EXPECT_FALSE(configurations[j].is_match(min_adc_measurement));
      EXPECT_FALSE(configurations[j].is_match(max_adc_measurement));
    }
  }
}

TEST(BoardIdTest, TotalResistanceNearExpectedValue) {
  for (const auto& configuration : board_id_voltage_dividers()) {
    const float sum{configuration.high_resistor_value() + configuration.low_resistor_value()};
    EXPECT_LT(0.90f * VOLTAGE_DIVIDER_TOTAL_RESISTANCE, sum);
    EXPECT_LT(sum, 1.10f * VOLTAGE_DIVIDER_TOTAL_RESISTANCE);
  }
}

TEST(BoardIdTest, SourceImpedenceBelowThreshold) {
  // ST Micro recommends that the source impedance into an ADC on the STML4 series be less than 10k
  // to ensure that the ADC can measure the value quickly. We can use a higher threshold, since
  // speed doesn't matter in this situation, but this test uses the recommended threshold.
  // See "Table 78. Maximum ADC RAIN" in the datasheet for the STM32L452
  // https://www.st.com/resource/en/datasheet/stm32l452ce.pdf.
  for (const auto& configuration : board_id_voltage_dividers()) {
    EXPECT_LT(configuration.source_impedance(), 10'000.f);
  }
}

TEST(CanonicalBoardIdsTest, AllEnumValuesMatchConfiguration) {
  for (auto id : VALID_BOARD_IDS) {
    bool found_id{false};
    for (const auto& configuration : board_id_voltage_dividers()) {
      if (cast_to_underlying_type(id) == configuration.id()) {
        found_id = true;
        break;
      }
    }
    if (id != CanonicalBoardIds::UNKNOWN) {
      EXPECT_TRUE(found_id);
    } else {
      EXPECT_FALSE(found_id);
    }
  }
}

}  // namespace tvsc::hal2::board_identification
