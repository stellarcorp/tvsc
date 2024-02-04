#pragma once

#include "control/combined_parameter_domains.h"
#include "control/excluded_parameter_domains.h"
#include "control/parameter_domain.h"

namespace tvsc::control {

template <typename T, typename... Args>
std::unique_ptr<Parameter<T>> combine(Args... parameter_domains) {
  std::vector<std::unique_ptr<Parameter<T>>> v{};
  v.reserve(sizeof...(Args));
  (v.emplace_back(std::forward<Args>(parameter_domains)), ...);
  return std::unique_ptr<Parameter<T>>(new impl::CombinedParameters<T>(std::move(v)));
}

  template <typename T, typename... Args>
std::unique_ptr<Parameter<T>> exclude(std::unique_ptr<Parameter<T>>&& parameter_domain,
                                      Args... excluded) {
  std::vector<std::unique_ptr<Parameter<T>>> v{};
  v.reserve(sizeof...(Args));
  (v.emplace_back(std::forward<Args>(excluded)), ...);
  return std::unique_ptr<Parameter<T>>(new impl::ExcludedParameters<T>(std::move(parameter_domain), std::move(v)));
  }

}  // namespace tvsc::control
