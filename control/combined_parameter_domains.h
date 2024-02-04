#pragma once

#include <initializer_list>
#include <memory>
#include <vector>

#include "control/parameter_domain.h"

namespace tvsc::control {

namespace impl {

template <typename T>
class CombinedParameters final : public Parameter<T> {
 private:
  const std::vector<std::unique_ptr<Parameter<T>>> parameter_domains_;

 public:
  CombinedParameters(std::vector<std::unique_ptr<Parameter<T>>>&& parameter_domains)
      : parameter_domains_(std::move(parameter_domains)) {}

  CombinedParameters(CombinedParameters&& rhs) : parameter_domains_(std::move(rhs.parameter_domains_)) {}

  CombinedParameters& operator=(CombinedParameters&& rhs) {
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
