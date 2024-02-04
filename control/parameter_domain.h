#pragma once

#include <initializer_list>
#include <memory>
#include <vector>

#include "glog/logging.h"

namespace tvsc::control {

template <typename T>
class Parameter {
 public:
  virtual ~Parameter() { LOG(INFO) << "Parameter::~Parameter()"; }
  virtual bool is_allowed(const T& value) const = 0;
};

}  // namespace tvsc::control
