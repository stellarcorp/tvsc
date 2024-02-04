#pragma once

#include "control/combined_parameter_domains.h"
#include "control/excluded_parameter_domains.h"
#include "control/parameter_domain.h"

namespace tvsc::control {

template <typename T, typename... Args>
std::unique_ptr<ParameterDomain<T>> combine(Args... parameter_domains) {
  std::vector<std::unique_ptr<ParameterDomain<T>>> v{};
  v.reserve(sizeof...(Args));
  (v.emplace_back(std::forward<Args>(parameter_domains)), ...);
  return std::unique_ptr<ParameterDomain<T>>(new impl::CombinedParameterDomains<T>(std::move(v)));
}

  template <typename T, typename... Args>
std::unique_ptr<ParameterDomain<T>> exclude(std::unique_ptr<ParameterDomain<T>>&& parameter_domain,
                                      Args... excluded) {
  std::vector<std::unique_ptr<ParameterDomain<T>>> v{};
  v.reserve(sizeof...(Args));
  (v.emplace_back(std::forward<Args>(excluded)), ...);
  return std::unique_ptr<ParameterDomain<T>>(new impl::ExcludedParameterDomains<T>(std::move(parameter_domain), std::move(v)));
  }

}  // namespace tvsc::control
