#pragma once

#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "base/except.h"
#include "comms/radio/nanopb_proto/settings.pb.h"

namespace tvsc::comms::radio {

/**
 * Generate a Value that is a range of float values from a to b. The value of the inclusivity
 * parameter dictates which of the endpoints is included.
 */
inline tvsc_comms_radio_nano_Value float_range(float a, float b,
                                         tvsc_comms_radio_nano_RangeInclusivity inclusivity =
                                             tvsc_comms_radio_nano_RangeInclusivity_INCLUSIVE_INCLUSIVE) {
  tvsc_comms_radio_nano_Value result{};
  result.ranged_count = 1;
  result.ranged[0].which_value = tvsc_comms_radio_nano_RangedValue_float_range_tag;
  result.ranged[0].value.float_range = {a, b, inclusivity};
  return result;
}

/**
 * Generate a Value that is a range of 32-bit integer values from a to b. The value of the
 * inclusivity parameter dictates which of the endpoints is included.
 */
inline tvsc_comms_radio_nano_Value int32_range(int32_t a, int32_t b,
                                         tvsc_comms_radio_nano_RangeInclusivity inclusivity =
                                             tvsc_comms_radio_nano_RangeInclusivity_INCLUSIVE_INCLUSIVE) {
  tvsc_comms_radio_nano_Value result{};
  result.ranged_count = 1;
  result.ranged[0].which_value = tvsc_comms_radio_nano_RangedValue_int32_range_tag;
  result.ranged[0].value.int32_range = {a, b, inclusivity};
  return result;
}

/**
 * Generate a Value that is a range of 64-bit integer values from a to b. The value of the
 * inclusivity parameter dictates which of the endpoints is included.
 */
inline tvsc_comms_radio_nano_Value int64_range(int64_t a, int64_t b,
                                         tvsc_comms_radio_nano_RangeInclusivity inclusivity =
                                             tvsc_comms_radio_nano_RangeInclusivity_INCLUSIVE_INCLUSIVE) {
  tvsc_comms_radio_nano_Value result{};
  result.ranged_count = 1;
  result.ranged[0].which_value = tvsc_comms_radio_nano_RangedValue_int64_range_tag;
  result.ranged[0].value.int64_range = {a, b, inclusivity};
  return result;
}

/**
 * Generate a Value that is the set of discrete enumerated values.
 */
// Value from a list of enum values.
template <typename T>
inline tvsc_comms_radio_nano_Value enumerated(std::initializer_list<T> values) {
  tvsc_comms_radio_nano_Value result{};
  result.discrete_count = values.size();

  int i{};
  for (const auto value : values) {
    result.discrete[i].which_value = tvsc_comms_radio_nano_DiscreteValue_int32_value_tag;
    result.discrete[i].value.int32_value = value;
    ++i;
  }

  return result;
}

// Value from a list of int32 values.
template <>
inline tvsc_comms_radio_nano_Value enumerated<int32_t>(std::initializer_list<int32_t> values) {
  tvsc_comms_radio_nano_Value result{};
  result.discrete_count = values.size();

  int i{};
  for (const auto value : values) {
    result.discrete[i].which_value = tvsc_comms_radio_nano_DiscreteValue_int32_value_tag;
    result.discrete[i].value.int32_value = value;
    ++i;
  }

  return result;
}

// Value from a list of int64 values.
template <>
inline tvsc_comms_radio_nano_Value enumerated<int64_t>(std::initializer_list<int64_t> values) {
  tvsc_comms_radio_nano_Value result{};
  result.discrete_count = values.size();

  int i{};
  for (const auto value : values) {
    result.discrete[i].which_value = tvsc_comms_radio_nano_DiscreteValue_int64_value_tag;
    result.discrete[i].value.int64_value = value;
    ++i;
  }

  return result;
}

// Value from a list of float values.
template <>
inline tvsc_comms_radio_nano_Value enumerated<float>(std::initializer_list<float> values) {
  tvsc_comms_radio_nano_Value result{};
  result.discrete_count = values.size();

  int i{};
  for (const auto value : values) {
    result.discrete[i].which_value = tvsc_comms_radio_nano_DiscreteValue_float_value_tag;
    result.discrete[i].value.float_value = value;
    ++i;
  }

  return result;
}

/**
 * Extract the value from a DiscreteValue as a native type.
 */
// DiscreteValue translated into an enum value.
template <typename T>
inline T as(const tvsc_comms_radio_nano_DiscreteValue& value) {
  int32_t int_value = as<int32_t>(value);
  return static_cast<T>(int_value);
}

template <>
inline int8_t as<int8_t>(const tvsc_comms_radio_nano_DiscreteValue& value) {
  if (value.which_value == tvsc_comms_radio_nano_DiscreteValue_int32_value_tag) {
    return value.value.int32_value;
  } else {
    except<std::domain_error>("Attempt to translate DiscreteValue to inappropriate type (int8_t)");
  }
}

template <>
inline int16_t as<int16_t>(const tvsc_comms_radio_nano_DiscreteValue& value) {
  if (value.which_value == tvsc_comms_radio_nano_DiscreteValue_int32_value_tag) {
    return value.value.int32_value;
  } else {
    except<std::domain_error>("Attempt to translate DiscreteValue to inappropriate type (int16_t)");
  }
}

template <>
inline int32_t as<int32_t>(const tvsc_comms_radio_nano_DiscreteValue& value) {
  if (value.which_value == tvsc_comms_radio_nano_DiscreteValue_int32_value_tag) {
    return value.value.int32_value;
  } else {
    except<std::domain_error>("Attempt to translate DiscreteValue to inappropriate type (int32_t)");
  }
}

template <>
inline int64_t as<int64_t>(const tvsc_comms_radio_nano_DiscreteValue& value) {
  if (value.which_value == tvsc_comms_radio_nano_DiscreteValue_int64_value_tag) {
    return value.value.int64_value;
  } else {
    except<std::domain_error>("Attempt to translate DiscreteValue to inappropriate type (int64_t)");
  }
}

template <>
inline uint8_t as<uint8_t>(const tvsc_comms_radio_nano_DiscreteValue& value) {
  if (value.which_value == tvsc_comms_radio_nano_DiscreteValue_int32_value_tag) {
    return value.value.int32_value;
  } else {
    except<std::domain_error>("Attempt to translate DiscreteValue to inappropriate type (uint8_t)");
  }
}

template <>
inline uint16_t as<uint16_t>(const tvsc_comms_radio_nano_DiscreteValue& value) {
  if (value.which_value == tvsc_comms_radio_nano_DiscreteValue_int32_value_tag) {
    return value.value.int32_value;
  } else {
    except<std::domain_error>(
        "Attempt to translate DiscreteValue to inappropriate type (uint16_t)");
  }
}

template <>
inline uint32_t as<uint32_t>(const tvsc_comms_radio_nano_DiscreteValue& value) {
  if (value.which_value == tvsc_comms_radio_nano_DiscreteValue_int32_value_tag) {
    return value.value.int32_value;
  } else {
    except<std::domain_error>(
        "Attempt to translate DiscreteValue to inappropriate type (uint32_t)");
  }
}

template <>
inline uint64_t as<uint64_t>(const tvsc_comms_radio_nano_DiscreteValue& value) {
  if (value.which_value == tvsc_comms_radio_nano_DiscreteValue_int64_value_tag) {
    return value.value.int64_value;
  } else {
    except<std::domain_error>(
        "Attempt to translate DiscreteValue to inappropriate type (uint64_t)");
  }
}

template <>
inline float as<float>(const tvsc_comms_radio_nano_DiscreteValue& value) {
  if (value.which_value == tvsc_comms_radio_nano_DiscreteValue_float_value_tag) {
    return value.value.float_value;
  } else {
    except<std::domain_error>("Attempt to translate DiscreteValue to inappropriate type (float)");
  }
}

/**
 * Generate a DiscreteValue from a native type.
 */
template <typename T>
inline tvsc_comms_radio_nano_DiscreteValue as_discrete_value(T value) {
  tvsc_comms_radio_nano_DiscreteValue discrete{};
  auto v{static_cast<typename std::underlying_type<T>::type>(value)};
  discrete.which_value = tvsc_comms_radio_nano_DiscreteValue_int32_value_tag;
  discrete.value.int32_value = v;
  return discrete;
}

template <>
inline tvsc_comms_radio_nano_DiscreteValue as_discrete_value<int8_t>(int8_t value) {
  tvsc_comms_radio_nano_DiscreteValue discrete{};
  discrete.which_value = tvsc_comms_radio_nano_DiscreteValue_int32_value_tag;
  discrete.value.int32_value = value;
  return discrete;
}

template <>
inline tvsc_comms_radio_nano_DiscreteValue as_discrete_value<int16_t>(int16_t value) {
  tvsc_comms_radio_nano_DiscreteValue discrete{};
  discrete.which_value = tvsc_comms_radio_nano_DiscreteValue_int32_value_tag;
  discrete.value.int32_value = value;
  return discrete;
}

template <>
inline tvsc_comms_radio_nano_DiscreteValue as_discrete_value<int32_t>(int32_t value) {
  tvsc_comms_radio_nano_DiscreteValue discrete{};
  discrete.which_value = tvsc_comms_radio_nano_DiscreteValue_int32_value_tag;
  discrete.value.int32_value = value;
  return discrete;
}

template <>
inline tvsc_comms_radio_nano_DiscreteValue as_discrete_value<int64_t>(int64_t value) {
  tvsc_comms_radio_nano_DiscreteValue discrete{};
  discrete.which_value = tvsc_comms_radio_nano_DiscreteValue_int64_value_tag;
  discrete.value.int64_value = value;
  return discrete;
}

template <>
inline tvsc_comms_radio_nano_DiscreteValue as_discrete_value<uint8_t>(uint8_t value) {
  tvsc_comms_radio_nano_DiscreteValue discrete{};
  discrete.which_value = tvsc_comms_radio_nano_DiscreteValue_int32_value_tag;
  discrete.value.int32_value = value;
  return discrete;
}

template <>
inline tvsc_comms_radio_nano_DiscreteValue as_discrete_value<uint16_t>(uint16_t value) {
  tvsc_comms_radio_nano_DiscreteValue discrete{};
  discrete.which_value = tvsc_comms_radio_nano_DiscreteValue_int32_value_tag;
  discrete.value.int32_value = value;
  return discrete;
}

template <>
inline tvsc_comms_radio_nano_DiscreteValue as_discrete_value<uint32_t>(uint32_t value) {
  tvsc_comms_radio_nano_DiscreteValue discrete{};
  discrete.which_value = tvsc_comms_radio_nano_DiscreteValue_int32_value_tag;
  discrete.value.int32_value = value;
  return discrete;
}

template <>
inline tvsc_comms_radio_nano_DiscreteValue as_discrete_value<uint64_t>(uint64_t value) {
  tvsc_comms_radio_nano_DiscreteValue discrete{};
  discrete.which_value = tvsc_comms_radio_nano_DiscreteValue_int64_value_tag;
  discrete.value.int64_value = value;
  return discrete;
}

template <>
inline tvsc_comms_radio_nano_DiscreteValue as_discrete_value<float>(float value) {
  tvsc_comms_radio_nano_DiscreteValue discrete{};
  discrete.which_value = tvsc_comms_radio_nano_DiscreteValue_float_value_tag;
  discrete.value.float_value = value;
  return discrete;
}

/**
 * Are two DiscreteValues equivalent.
 *
 * TODO(james): Determine if we should increase the flexibility here. Should we allow 1 (signed int)
 * to be equivalent to 1L (signed long int)? 1UL vs 1L? etc.
 *
 * TODO(james): Add an epsilon concept for evaluating equivalence of floating point numbers.
 */
inline bool are_equivalent(const tvsc_comms_radio_nano_DiscreteValue& lhs,
                           const tvsc_comms_radio_nano_DiscreteValue& rhs) {
  if (lhs.which_value == rhs.which_value) {
    if (lhs.which_value == tvsc_comms_radio_nano_DiscreteValue_int32_value_tag) {
      return lhs.value.int32_value == rhs.value.int32_value;
    } else if (lhs.which_value == tvsc_comms_radio_nano_DiscreteValue_int64_value_tag) {
      return lhs.value.int64_value == rhs.value.int64_value;
    } else if (lhs.which_value == tvsc_comms_radio_nano_DiscreteValue_float_value_tag) {
      return lhs.value.float_value == rhs.value.float_value;
    }
  }
  return false;
}

/**
 * Does the range contain the value?
 *
 * Currently, this requires that the value and the range have the same type. Only int32 values can
 * be in an int32_range, for example.
 */
inline bool range_contains(const tvsc_comms_radio_nano_RangedValue& range,
                           const tvsc_comms_radio_nano_DiscreteValue& value) {
  if (value.which_value == tvsc_comms_radio_nano_DiscreteValue_int32_value_tag) {
    if (range.which_value == tvsc_comms_radio_nano_RangedValue_int32_range_tag) {
      const auto& typed_range = range.value.int32_range;
      switch (typed_range.inclusive) {
        case tvsc_comms_radio_nano_RangeInclusivity_INCLUSIVE_INCLUSIVE:
          return typed_range.min <= value.value.int32_value &&
                 value.value.int32_value <= typed_range.max;
        case tvsc_comms_radio_nano_RangeInclusivity_INCLUSIVE_EXCLUSIVE:
          return typed_range.min <= value.value.int32_value &&
                 value.value.int32_value < typed_range.max;
        case tvsc_comms_radio_nano_RangeInclusivity_EXCLUSIVE_INCLUSIVE:
          return typed_range.min < value.value.int32_value &&
                 value.value.int32_value <= typed_range.max;
        case tvsc_comms_radio_nano_RangeInclusivity_EXCLUSIVE_EXCLUSIVE:
          return typed_range.min < value.value.int32_value &&
                 value.value.int32_value < typed_range.max;
      }
    } else {
      return false;
    }
  } else if (value.which_value == tvsc_comms_radio_nano_DiscreteValue_int64_value_tag) {
    if (range.which_value == tvsc_comms_radio_nano_RangedValue_int64_range_tag) {
      const auto& typed_range = range.value.int64_range;
      switch (typed_range.inclusive) {
        case tvsc_comms_radio_nano_RangeInclusivity_INCLUSIVE_INCLUSIVE:
          return typed_range.min <= value.value.int64_value &&
                 value.value.int64_value <= typed_range.max;
        case tvsc_comms_radio_nano_RangeInclusivity_INCLUSIVE_EXCLUSIVE:
          return typed_range.min <= value.value.int64_value &&
                 value.value.int64_value < typed_range.max;
        case tvsc_comms_radio_nano_RangeInclusivity_EXCLUSIVE_INCLUSIVE:
          return typed_range.min < value.value.int64_value &&
                 value.value.int64_value <= typed_range.max;
        case tvsc_comms_radio_nano_RangeInclusivity_EXCLUSIVE_EXCLUSIVE:
          return typed_range.min < value.value.int64_value &&
                 value.value.int64_value < typed_range.max;
      }
    } else {
      return false;
    }
  } else if (value.which_value == tvsc_comms_radio_nano_DiscreteValue_float_value_tag) {
    if (range.which_value == tvsc_comms_radio_nano_RangedValue_float_range_tag) {
      const auto& typed_range = range.value.float_range;
      switch (typed_range.inclusive) {
        case tvsc_comms_radio_nano_RangeInclusivity_INCLUSIVE_INCLUSIVE:
          return typed_range.min <= value.value.float_value &&
                 value.value.float_value <= typed_range.max;
        case tvsc_comms_radio_nano_RangeInclusivity_INCLUSIVE_EXCLUSIVE:
          return typed_range.min <= value.value.float_value &&
                 value.value.float_value < typed_range.max;
        case tvsc_comms_radio_nano_RangeInclusivity_EXCLUSIVE_INCLUSIVE:
          return typed_range.min < value.value.float_value &&
                 value.value.float_value <= typed_range.max;
        case tvsc_comms_radio_nano_RangeInclusivity_EXCLUSIVE_EXCLUSIVE:
          return typed_range.min < value.value.float_value &&
                 value.value.float_value < typed_range.max;
      }
    } else {
      return false;
    }
  }
  return false;
}

/**
 * Is the given value in the set of allowed_values?
 */
inline bool is_valid_setting(const tvsc_comms_radio_nano_Value& allowed_values,
                             const tvsc_comms_radio_nano_DiscreteValue& value) {
  for (int i = 0; i < allowed_values.discrete_count; ++i) {
    if (are_equivalent(allowed_values.discrete[i], value)) {
      return true;
    }
  }
  for (int i = 0; i < allowed_values.ranged_count; ++i) {
    if (range_contains(allowed_values.ranged[i], value)) {
      return true;
    }
  }
  return false;
}

}  // namespace tvsc::comms::radio
