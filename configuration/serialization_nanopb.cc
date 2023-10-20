#include <cstdint>

#include "configuration/nanopb_proto/system_definition.pb.h"
#include "configuration/serialization.h"
#include "configuration/system_configuration.h"
#include "configuration/system_definition.h"

namespace tvsc::configuration {

bool encode(const SystemDefinition& defintion, uint8_t* buf, size_t& size) {}

bool decode(SystemDefinition& defintion, const uint8_t* buf, size_t size) {}

bool encode(const SystemConfiguration& defintion, uint8_t* buf, size_t& size) {}

bool decode(SystemConfiguration& defintion, const uint8_t* buf, size_t size) {}

}  // namespace tvsc::configuration
