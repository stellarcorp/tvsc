#pragma once

#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "radio/settings.pb.h"
#include "random/random.h"

namespace tvsc::radio {

/**
 * Generate a Value that is a range of float values from a to b. The value of the inclusivity
 * parameter dictates which of the endpoints is included.
 */
Value float_range(float a, float b,
                  RangeInclusivity inclusivity = RangeInclusivity::INCLUSIVE_INCLUSIVE) {
  Value result{};
  auto* range = result.add_ranged();
  auto* float_range = range->mutable_float_range();
  float_range->set_min(a);
  float_range->set_max(b);
  float_range->set_inclusive(inclusivity);
  return result;
}

/**
 * Generate a Value that is a range of 32-bit integer values from a to b. The value of the
 * inclusivity parameter dictates which of the endpoints is included.
 */
Value int32_range(int32_t a, int32_t b,
                  RangeInclusivity inclusivity = RangeInclusivity::INCLUSIVE_INCLUSIVE) {
  Value result{};
  auto* range = result.add_ranged();
  auto* int_range = range->mutable_int32_range();
  int_range->set_min(a);
  int_range->set_max(b);
  int_range->set_inclusive(inclusivity);
  return result;
}

/**
 * Generate a Value that is a range of 64-bit integer values from a to b. The value of the
 * inclusivity parameter dictates which of the endpoints is included.
 */
Value int64_range(int64_t a, int64_t b,
                  RangeInclusivity inclusivity = RangeInclusivity::INCLUSIVE_INCLUSIVE) {
  Value result{};
  auto* range = result.add_ranged();
  auto* int_range = range->mutable_int64_range();
  int_range->set_min(a);
  int_range->set_max(b);
  int_range->set_inclusive(inclusivity);
  return result;
}

/**
 * Generate a Value that is the set of discrete enumerated values.
 */
template <typename T>
Value enumerated(std::initializer_list<T> values) {
  Value result{};
  for (auto value : values) {
    DiscreteValue* discrete = result.add_discrete();
    auto v{static_cast<typename std::underlying_type<T>::type>(value)};
    discrete->set_int32_value(v);
  }
  return result;
}

template <>
Value enumerated<int8_t>(std::initializer_list<int8_t> values) {
  Value result{};
  for (auto value : values) {
    DiscreteValue* discrete = result.add_discrete();
    discrete->set_int32_value(value);
  }
  return result;
}

template <>
Value enumerated<int16_t>(std::initializer_list<int16_t> values) {
  Value result{};
  for (auto value : values) {
    DiscreteValue* discrete = result.add_discrete();
    discrete->set_int32_value(value);
  }
  return result;
}

template <>
Value enumerated<int32_t>(std::initializer_list<int32_t> values) {
  Value result{};
  for (auto value : values) {
    DiscreteValue* discrete = result.add_discrete();
    discrete->set_int32_value(value);
  }
  return result;
}

template <>
Value enumerated<int64_t>(std::initializer_list<int64_t> values) {
  Value result{};
  for (auto value : values) {
    DiscreteValue* discrete = result.add_discrete();
    discrete->set_int64_value(value);
  }
  return result;
}

/**
 * Extract the value from a DiscreteValue as a native type.
 */
template <typename T>
T as(const DiscreteValue& value);

template <>
int8_t as<int8_t>(const DiscreteValue& value) {
  return static_cast<int8_t>(value.int32_value());
}

template <>
uint8_t as<uint8_t>(const DiscreteValue& value) {
  return static_cast<uint8_t>(value.int32_value());
}

template <>
int16_t as<int16_t>(const DiscreteValue& value) {
  return static_cast<int16_t>(value.int32_value());
}

template <>
uint16_t as<uint16_t>(const DiscreteValue& value) {
  return static_cast<uint16_t>(value.int32_value());
}

template <>
int32_t as<int32_t>(const DiscreteValue& value) {
  return static_cast<int32_t>(value.int32_value());
}

template <>
uint32_t as<uint32_t>(const DiscreteValue& value) {
  return static_cast<uint32_t>(value.int32_value());
}

template <>
int64_t as<int64_t>(const DiscreteValue& value) {
  return static_cast<int64_t>(value.int64_value());
}

template <>
uint64_t as<uint64_t>(const DiscreteValue& value) {
  return static_cast<uint64_t>(value.int64_value());
}

template <>
float as<float>(const DiscreteValue& value) {
  return value.float_value();
}

/**
 * Generate a DiscreteValue from a native type.
 */
template <typename T>
DiscreteValue as_discrete_value(T value) {
  DiscreteValue discrete{};
  auto v{static_cast<typename std::underlying_type<T>::type>(value)};
  discrete.set_int32_value(v);
  return discrete;
}

template <>
DiscreteValue as_discrete_value<int32_t>(int32_t value) {
  DiscreteValue discrete{};
  discrete.set_int32_value(value);
  return discrete;
}

template <>
DiscreteValue as_discrete_value<int64_t>(int64_t value) {
  DiscreteValue discrete{};
  discrete.set_int64_value(value);
  return discrete;
}

template <>
DiscreteValue as_discrete_value<float>(float value) {
  DiscreteValue discrete{};
  discrete.set_float_value(value);
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
bool are_equivalent(const DiscreteValue& lhs, const DiscreteValue& rhs) {
  if (lhs.has_int32_value()) {
    if (rhs.has_int32_value()) {
      return lhs.int32_value() == rhs.int32_value();
    } else {
      return false;
    }
  } else if (lhs.has_int64_value()) {
    if (rhs.has_int64_value()) {
      return lhs.int64_value() == rhs.int64_value();
    } else {
      return false;
    }
  } else if (lhs.has_float_value()) {
    if (rhs.has_float_value()) {
      return lhs.float_value() == rhs.float_value();
    } else {
      return false;
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
bool range_contains(const RangedValue& range, const DiscreteValue& value) {
  if (value.has_int32_value()) {
    if (range.has_int32_range()) {
      const auto& typed_range = range.int32_range();
      switch (typed_range.inclusive()) {
        case RangeInclusivity::INCLUSIVE_INCLUSIVE:
          return typed_range.min() <= value.int32_value() &&
                 value.int32_value() <= typed_range.max();
        case RangeInclusivity::INCLUSIVE_EXCLUSIVE:
          return typed_range.min() <= value.int32_value() &&
                 value.int32_value() < typed_range.max();
        case RangeInclusivity::EXCLUSIVE_INCLUSIVE:
          return typed_range.min() < value.int32_value() &&
                 value.int32_value() <= typed_range.max();
        case RangeInclusivity::EXCLUSIVE_EXCLUSIVE:
          return typed_range.min() < value.int32_value() && value.int32_value() < typed_range.max();
      }
    } else {
      return false;
    }
  } else if (value.has_int64_value()) {
    if (range.has_int64_range()) {
      const auto& typed_range = range.int64_range();
      switch (typed_range.inclusive()) {
        case RangeInclusivity::INCLUSIVE_INCLUSIVE:
          return typed_range.min() <= value.int64_value() &&
                 value.int64_value() <= typed_range.max();
        case RangeInclusivity::INCLUSIVE_EXCLUSIVE:
          return typed_range.min() <= value.int64_value() &&
                 value.int64_value() < typed_range.max();
        case RangeInclusivity::EXCLUSIVE_INCLUSIVE:
          return typed_range.min() < value.int64_value() &&
                 value.int64_value() <= typed_range.max();
        case RangeInclusivity::EXCLUSIVE_EXCLUSIVE:
          return typed_range.min() < value.int64_value() && value.int64_value() < typed_range.max();
      }
    } else {
      return false;
    }
  } else if (value.has_float_value()) {
    if (range.has_float_range()) {
      const auto& typed_range = range.float_range();
      switch (typed_range.inclusive()) {
        case RangeInclusivity::INCLUSIVE_INCLUSIVE:
          return typed_range.min() <= value.float_value() &&
                 value.float_value() <= typed_range.max();
        case RangeInclusivity::INCLUSIVE_EXCLUSIVE:
          return typed_range.min() <= value.float_value() &&
                 value.float_value() < typed_range.max();
        case RangeInclusivity::EXCLUSIVE_INCLUSIVE:
          return typed_range.min() < value.float_value() &&
                 value.float_value() <= typed_range.max();
        case RangeInclusivity::EXCLUSIVE_EXCLUSIVE:
          return typed_range.min() < value.float_value() && value.float_value() < typed_range.max();
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
bool is_valid_setting(const Value& allowed_values, const DiscreteValue& value) {
  for (const auto& discrete : allowed_values.discrete()) {
    if (are_equivalent(discrete, value)) {
      return true;
    }
  }
  for (const auto& range : allowed_values.ranged()) {
    if (range_contains(range, value)) {
      return true;
    }
  }
  return false;
}

}  // namespace tvsc::radio
