#pragma once

#include <initializer_list>
#include <memory>
#include <vector>

#include "control/parameter.h"

namespace tvsc::control {

namespace impl {

template <typename T>
class CombinedParameters final : public Parameter<T> {
 private:
  const std::vector<std::unique_ptr<Parameter<T>>> parameters_;

 public:
  CombinedParameters(std::vector<std::unique_ptr<Parameter<T>>>&& parameters)
      : parameters_(std::move(parameters)) {}

  CombinedParameters(CombinedParameters&& rhs) : parameters_(std::move(rhs.parameters_)) {}

  CombinedParameters& operator=(CombinedParameters&& rhs) {
    parameters_ = std::move(rhs.parameters_);
    return *this;
  }

  bool is_allowed(const T& value) const override {
    for (const auto& p : parameters_) {
      if (p->is_allowed(value)) {
        return true;
      }
    }
    return false;
  }
};

}  // namespace impl

}  // namespace tvsc::control
