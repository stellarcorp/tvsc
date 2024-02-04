#pragma once

#include <initializer_list>
#include <memory>
#include <vector>

namespace tvsc::control {

template <typename T>
class ParameterDomain {
 public:
  virtual ~ParameterDomain() = default;
  virtual bool in_domain(const T& value) const = 0;
  virtual double size() const = 0;
};

}  // namespace tvsc::control
