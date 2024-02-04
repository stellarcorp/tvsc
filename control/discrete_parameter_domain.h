#pragma once

#include <utility>

#include "control/parameter_domain.h"

namespace tvsc::control {

template <typename T>
class DiscreteParameterDomain final : public ParameterDomain<T> {
 private:
  T value_{};

 public:
  DiscreteParameterDomain() = default;

  DiscreteParameterDomain(const T& value) : value_(value) {}
  DiscreteParameterDomain(T&& value) : value_(std::move(value)) {}

  DiscreteParameterDomain(const DiscreteParameterDomain& rhs) : value_(rhs.value_) {}
  DiscreteParameterDomain(DiscreteParameterDomain&& rhs) : value_(std::move(rhs.value_)) {}

  DiscreteParameterDomain& operator=(const T& value) {
    value_ = value;
    return *this;
  }
  DiscreteParameterDomain& operator=(T&& value) {
    value_ = std::move(value);
    return *this;
  }
  DiscreteParameterDomain& operator=(const DiscreteParameterDomain& rhs) {
    value_ = rhs.value_;
    return *this;
  }
  DiscreteParameterDomain& operator=(DiscreteParameterDomain&& rhs) {
    value_ = std::move(rhs.value_);
    return *this;
  }

  bool in_domain(const T& value) const override { return value_ == value; }

  double size() const override {
    return 1;
  }
};

}  // namespace tvsc::control
