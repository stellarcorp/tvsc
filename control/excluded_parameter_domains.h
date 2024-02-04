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

  bool in_domain(const T& value) const override {
    if (!parameter_domain_->in_domain(value)) {
      return false;
    }
    for (const auto& p : excluded_) {
      if (p->in_domain(value)) {
        return false;
      }
    }
    return true;
  }

  double size() const override {
    double result{parameter_domain_->size()};
    // Note that we purposefully double count overlaps in ranged parameter domains here. See the
    // note in the CombinedParameterDomains class for a bit more insight.
    for (const auto& p : excluded_) {
      result -= p->size();
    }
    return result;
  }
};

}  // namespace impl

}  // namespace tvsc::control
