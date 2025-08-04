#pragma once

#include <cstdint>

#include "message/message.h"

namespace tvsc::message {

template <typename ElementT>
class Processor {
 public:
  using ElementType = ElementT;

  virtual ~Processor() {}

  virtual bool process(const ElementType& element) = 0;
};

}  // namespace tvsc::message
