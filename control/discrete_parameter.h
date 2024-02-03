#pragma once

#include <utility>

#include "control/parameter.h"

namespace tvsc::control {

template <typename T>
class DiscreteParameter final : public Parameter<T> {
 private:
  T value_{};

 public:
  DiscreteParameter() = default;

  DiscreteParameter(const T& value) : value_(value) {}
  DiscreteParameter(T&& value) : value_(std::move(value)) {}

  DiscreteParameter(const DiscreteParameter& rhs) : value_(rhs.value_) {}
  DiscreteParameter(DiscreteParameter&& rhs) : value_(std::move(rhs.value_)) {}

  DiscreteParameter& operator=(const T& value) {
    value_ = value;
    return *this;
  }
  DiscreteParameter& operator=(T&& value) {
    value_ = std::move(value);
    return *this;
  }
  DiscreteParameter& operator=(const DiscreteParameter& rhs) {
    value_ = rhs.value_;
    return *this;
  }
  DiscreteParameter& operator=(DiscreteParameter&& rhs) {
    value_ = std::move(rhs.value_);
    return *this;
  }

  bool is_allowed(const T& value) const override { return value_ == value; }
};

}  // namespace tvsc::control
