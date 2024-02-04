#pragma once

#include "control/combined_parameters.h"
#include "control/excluded_parameters.h"
#include "control/parameter.h"

namespace tvsc::control {

template <typename T, typename... Args>
std::unique_ptr<Parameter<T>> combine(Args... parameters) {
  std::vector<std::unique_ptr<Parameter<T>>> v{};
  v.reserve(sizeof...(Args));
  (v.emplace_back(std::forward<Args>(parameters)), ...);
  return std::unique_ptr<Parameter<T>>(new impl::CombinedParameters<T>(std::move(v)));
}

  template <typename T, typename... Args>
std::unique_ptr<Parameter<T>> exclude(std::unique_ptr<Parameter<T>>&& parameter,
                                      Args... excluded) {
  std::vector<std::unique_ptr<Parameter<T>>> v{};
  v.reserve(sizeof...(Args));
  (v.emplace_back(std::forward<Args>(excluded)), ...);
  return std::unique_ptr<Parameter<T>>(new impl::ExcludedParameters<T>(std::move(parameter), std::move(v)));
  }

}  // namespace tvsc::control
