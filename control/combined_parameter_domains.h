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

  bool in_domain(const T& value) const override {
    for (const auto& p : parameter_domains_) {
      if (p->in_domain(value)) {
        return true;
      }
    }
    return false;
  }

  double size() const override {
    // We purposefully double count overlaps in combined ranged parameter domains. This is done for
    // a number of reasons.
    // 1. It's simpler than trying to consider how the ranges overlap. We could introduce
    // functionality to merge / heal overlapping ranges, but so far, it is not needed, except for
    // maybe in this size computation.
    // 2. We assume, correctly or not, that the overlap is meaningful, or it would have been
    // excluded before the ranges were created. Again, we could introduce a feature to merge
    // overlapping ranges, if it becomes necessary.
    double result{};
    for (const auto& p : parameter_domains_) {
      result += p->size();
    }
    return result;
  }
};

}  // namespace impl

}  // namespace tvsc::control
