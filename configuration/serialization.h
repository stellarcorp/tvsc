#pragma once

#include "configuration/allowed_values.h"
#include "configuration/system_configuration.h"
#include "configuration/system_definition.h"
#include "configuration/types.h"

namespace tvsc::configuration {

namespace internal {

// Internal functions exposed for testing.
bool encode(const ValueRange<int32_t>& value, uint8_t* buf, size_t& size);
bool decode(ValueRange<int32_t>& value, const uint8_t* buf, size_t size);

bool encode(const ValueRange<int64_t>& value, uint8_t* buf, size_t& size);
bool decode(ValueRange<int64_t>& value, const uint8_t* buf, size_t size);

bool encode(const ValueRange<float>& value, uint8_t* buf, size_t& size);
bool decode(ValueRange<float>& value, const uint8_t* buf, size_t size);

bool encode(const ValueRange<double>& value, uint8_t* buf, size_t& size);
bool decode(ValueRange<double>& value, const uint8_t* buf, size_t size);

bool encode(const DiscreteValue& value, uint8_t* buf, size_t& size);
bool decode(DiscreteValue& value, const uint8_t* buf, size_t size);

bool encode(const RangedValue& value, uint8_t* buf, size_t& size);
bool decode(RangedValue& value, const uint8_t* buf, size_t size);

bool encode(const AllowedValues& value, uint8_t* buf, size_t& size);
bool decode(AllowedValues& values, const uint8_t* buf, size_t size);

bool encode(const Function& function, uint8_t* buf, size_t& size);
bool decode(Function& function, const uint8_t* buf, size_t size);

bool encode(const Property& property, uint8_t* buf, size_t& size);
bool decode(Property& property, const uint8_t* buf, size_t size);

bool encode(const Setting& setting, uint8_t* buf, size_t& size);
bool decode(Setting& setting, const uint8_t* buf, size_t size);

}  // namespace internal

bool encode(const SystemDefinition& definition, uint8_t* buf, size_t& size);
bool decode(SystemDefinition& definition, const uint8_t* buf, size_t size);

bool encode(const SystemConfiguration& configuration, uint8_t* buf, size_t& size);
bool decode(SystemConfiguration& configuration, const uint8_t* buf, size_t size);

}  // namespace tvsc::configuration
