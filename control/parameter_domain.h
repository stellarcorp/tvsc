#pragma once

#include <initializer_list>
#include <memory>
#include <vector>

namespace tvsc::control {

template <typename T>
class ParameterDomain {
 public:
  virtual ~ParameterDomain() = default;
  virtual bool is_allowed(const T& value) const = 0;
};

}  // namespace tvsc::control
