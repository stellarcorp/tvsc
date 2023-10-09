#pragma once

#include "configuration/system_configuration.h"
#include "configuration/system_definition.h"

namespace tvsc::configuration {

  bool encode(const SystemDefinition& defintion, char* buf, size_t& size);
  bool decode(SystemDefinition& defintion, const char* buf, size_t size);

  bool encode(const SystemConfiguration& defintion, char* buf, size_t& size);
  bool decode(SystemConfiguration& defintion, const char* buf, size_t size);
  
}
