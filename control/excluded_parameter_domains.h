#pragma once

#include <initializer_list>
#include <memory>
#include <vector>

#include "control/parameter_domain.h"

namespace tvsc::control {

namespace impl {

template <typename T>
class ExcludedParameters final : public Parameter<T> {
 private:
  const std::unique_ptr<Parameter<T>> parameter_domain_;
  const std::vector<std::unique_ptr<Parameter<T>>> excluded_;

 public:
  ExcludedParameters(std::unique_ptr<Parameter<T>>&& parameter_domain, std::vector<std::unique_ptr<Parameter<T>>>&& excluded)
    : parameter_domain_(std::move(parameter_domain)), excluded_(std::move(excluded)) {}

  ExcludedParameters(ExcludedParameters&& rhs) : parameter_domain_(std::move(rhs.parameter_domain_)), excluded_(std::move(rhs.excluded_)) {}

  ExcludedParameters& operator=(ExcludedParameters&& rhs) {
    parameter_domain_ = std::move(rhs.parameter_domain_);
    excluded_ = std::move(rhs.excluded_);
    return *this;
  }

  bool is_allowed(const T& value) const override {
    if (!parameter_domain_->is_allowed(value)) {
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
