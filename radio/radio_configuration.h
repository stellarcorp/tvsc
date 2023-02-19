#pragma once

#include <string>
#include <vector>

#include "radio/radio.pb.h"

namespace tvsc::radio {

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

template <typename T>
Value enumerated(std::initializer_list<T> values) {
  Value result{};
  return result;
}

/**
 * This template defines an interface for configuring a radio. The specific radio type will be
 * driven by the template parameter.
 *
 * Note that we are trying to avoid an inheritance hierarchy here. We expect there to be many
 * different types of radios, even in the short term. We want a uniform way to interact with them
 * all, but we don't want the readability headaches that go with elaborate inheritance schemes.  Our
 * approach is to define a template and provide specializations of this template for each radio
 * type. These template specializations will be in their own header files and be locked away as
 * separate C++ libraries under Bazel. This scheme will also avoid any runtime overhead; the
 * compiler will only include the code that we need in each executable, and there will be no virtual
 * table indirections.
 */
template <typename DriverT>
class RadioConfiguration final {
 private:
  // Do not instantiate this one. Use a template specialization of this class.
  RadioConfiguration() {}

 public:
  uint64_t expanded_id() const { return 0L; }

  uint32_t id() const { return 0; }

  const std::string& name() const { return ""; }

  std::vector<Function> get_configurable_functions() const { return {}; }

  int32_t get_int32_value(Function function) const { return 0; }
  int64_t get_int64_value(Function function) const { return 0L; }
  float get_float_value(Function function) const { return 0.f; }

  void set_int32_value(Function function, int32_t value) const {}
  void set_int64_value(Function function, int64_t value) const {}
  void set_float_value(Function function, float value) const {}

  Value get_valid_values(Function function) const { return Value{}; }

  void commit_settings_changes() {}
  void abort_settings_changes() {}
};

}  // namespace tvsc::radio
