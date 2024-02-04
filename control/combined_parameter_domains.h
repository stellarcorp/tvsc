#pragma once

#include <initializer_list>
#include <memory>
#include <vector>

#include "control/parameter_domain.h"

namespace tvsc::control {

namespace impl {

template <typename T>
class CombinedParameterDomains final : public ParameterDomain<T> {
 private:
  const std::vector<std::unique_ptr<ParameterDomain<T>>> parameter_domains_;

 public:
  CombinedParameterDomains(std::vector<std::unique_ptr<ParameterDomain<T>>>&& parameter_domains)
      : parameter_domains_(std::move(parameter_domains)) {}

  CombinedParameterDomains(CombinedParameterDomains&& rhs)
      : parameter_domains_(std::move(rhs.parameter_domains_)) {}

  CombinedParameterDomains& operator=(CombinedParameterDomains&& rhs) {
    parameter_domains_ = std::move(rhs.parameter_domains_);
    return *this;
  }

  bool is_allowed(const T& value) const override {
    for (const auto& p : parameter_domains_) {
      if (p->is_allowed(value)) {
        return true;
      }
    }
    return false;
  }
};

}  // namespace impl

}  // namespace tvsc::control
