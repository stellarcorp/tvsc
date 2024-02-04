#pragma once

#include <initializer_list>
#include <memory>
#include <vector>

#include "control/parameter.h"

namespace tvsc::control {

namespace impl {

template <typename T>
class ExcludedParameters final : public Parameter<T> {
 private:
  const std::unique_ptr<Parameter<T>> parameter_;
  const std::vector<std::unique_ptr<Parameter<T>>> excluded_;

 public:
  ExcludedParameters(std::unique_ptr<Parameter<T>>&& parameter, std::vector<std::unique_ptr<Parameter<T>>>&& excluded)
    : parameter_(std::move(parameter)), excluded_(std::move(excluded)) {}

  ExcludedParameters(ExcludedParameters&& rhs) : parameter_(std::move(rhs.parameter_)), excluded_(std::move(rhs.excluded_)) {}

  ExcludedParameters& operator=(ExcludedParameters&& rhs) {
    parameter_ = std::move(rhs.parameter_);
    excluded_ = std::move(rhs.excluded_);
    return *this;
  }

  bool is_allowed(const T& value) const override {
    if (!parameter_->is_allowed(value)) {
      return false;
    }
    for (const auto& p : excluded_) {
      if (p->is_allowed(value)) {
        return false;
      }
    }
    return true;
  }
};

}  // namespace impl

}  // namespace tvsc::control
