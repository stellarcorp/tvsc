#pragma once

#include <initializer_list>
#include <memory>
#include <vector>

#include "control/parameter_domain.h"

namespace tvsc::control {

namespace impl {

template <typename T>
class ExcludedParameterDomains final : public ParameterDomain<T> {
 private:
  const std::unique_ptr<ParameterDomain<T>> parameter_domain_;
  const std::vector<std::unique_ptr<ParameterDomain<T>>> excluded_;

 public:
  ExcludedParameterDomains(std::unique_ptr<ParameterDomain<T>>&& parameter_domain,
                           std::vector<std::unique_ptr<ParameterDomain<T>>>&& excluded)
      : parameter_domain_(std::move(parameter_domain)), excluded_(std::move(excluded)) {}

  ExcludedParameterDomains(ExcludedParameterDomains&& rhs)
      : parameter_domain_(std::move(rhs.parameter_domain_)), excluded_(std::move(rhs.excluded_)) {}

  ExcludedParameterDomains& operator=(ExcludedParameterDomains&& rhs) {
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
