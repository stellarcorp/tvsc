#include <cstdint>
#include <string>
#include <type_traits>
#include <variant>

#include "configuration/proto/system_definition.pb.h"
#include "configuration/serialization.h"
#include "configuration/system_configuration.h"
#include "configuration/system_definition.h"
#include "configuration/types.h"

namespace tvsc::configuration {

void to_proto(const ValueRange<int32_t>& value, tvsc::configuration::proto::Int32Range& proto) {
  proto.set_min(value.first);
  proto.set_max(value.second);
}

void from_proto(const tvsc::configuration::proto::Int32Range& proto, ValueRange<int32_t>& value) {
  value.first = proto.min();
  value.second = proto.max();
}

void to_proto(const ValueRange<int64_t>& value, tvsc::configuration::proto::Int64Range& proto) {
  proto.set_min(value.first);
  proto.set_max(value.second);
}
void from_proto(const tvsc::configuration::proto::Int64Range& proto, ValueRange<int64_t>& value) {
  value.first = proto.min();
  value.second = proto.max();
}

void to_proto(const ValueRange<float>& value, tvsc::configuration::proto::FloatRange& proto) {
  proto.set_min(value.first);
  proto.set_max(value.second);
}
void from_proto(const tvsc::configuration::proto::FloatRange& proto, ValueRange<float>& value) {
  value.first = proto.min();
  value.second = proto.max();
}

void to_proto(const ValueRange<double>& value, tvsc::configuration::proto::DoubleRange& proto) {
  proto.set_min(value.first);
  proto.set_max(value.second);
}
void from_proto(const tvsc::configuration::proto::DoubleRange& proto, ValueRange<double>& value) {
  value.first = proto.min();
  value.second = proto.max();
}

void to_proto(const DiscreteValue& value, tvsc::configuration::proto::DiscreteValue& proto) {
  static_assert(std::variant_size_v<DiscreteValue> == 5,
                "DiscreteValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<int32_t, std::variant_alternative_t<0, DiscreteValue>>,
                "DiscreteValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<int64_t, std::variant_alternative_t<1, DiscreteValue>>,
                "DiscreteValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<float, std::variant_alternative_t<2, DiscreteValue>>,
                "DiscreteValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<double, std::variant_alternative_t<3, DiscreteValue>>,
                "DiscreteValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<std::string, std::variant_alternative_t<4, DiscreteValue>>,
                "DiscreteValue has been changed. Update this code block to match.");

  std::visit(
      [&proto](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, int32_t>) {
          proto.set_int32_value(arg);
        } else if constexpr (std::is_same_v<T, int64_t>) {
          proto.set_int64_value(arg);
        } else if constexpr (std::is_same_v<T, float>) {
          proto.set_float_value(arg);
        } else if constexpr (std::is_same_v<T, double>) {
          proto.set_double_value(arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
          proto.set_string_value(arg);
        }
      },
      value);
}

void from_proto(const tvsc::configuration::proto::DiscreteValue& proto, DiscreteValue& value) {
  static_assert(std::variant_size_v<DiscreteValue> == 5,
                "DiscreteValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<int32_t, std::variant_alternative_t<0, DiscreteValue>>,
                "DiscreteValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<int64_t, std::variant_alternative_t<1, DiscreteValue>>,
                "DiscreteValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<float, std::variant_alternative_t<2, DiscreteValue>>,
                "DiscreteValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<double, std::variant_alternative_t<3, DiscreteValue>>,
                "DiscreteValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<std::string, std::variant_alternative_t<4, DiscreteValue>>,
                "DiscreteValue has been changed. Update this code block to match.");

  if (proto.has_int32_value()) {
    value.emplace<int32_t>(proto.int32_value());
  } else if (proto.has_int64_value()) {
    value.emplace<int64_t>(proto.int64_value());
  } else if (proto.has_float_value()) {
    value.emplace<float>(proto.float_value());
  } else if (proto.has_double_value()) {
    value.emplace<double>(proto.double_value());
  } else if (proto.has_string_value()) {
    value.emplace<std::string>(proto.string_value());
  }
}

void to_proto(const RangedValue& value, tvsc::configuration::proto::RangedValue& proto) {
  static_assert(std::variant_size_v<RangedValue> == 4,
                "RangedValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<ValueRange<int32_t>, std::variant_alternative_t<0, RangedValue>>,
                "RangedValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<ValueRange<int64_t>, std::variant_alternative_t<1, RangedValue>>,
                "RangedValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<ValueRange<float>, std::variant_alternative_t<2, RangedValue>>,
                "RangedValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<ValueRange<double>, std::variant_alternative_t<3, RangedValue>>,
                "RangedValue has been changed. Update this code block to match.");

  std::visit(
      [&proto](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, ValueRange<int32_t>>) {
          proto.mutable_int32_range()->set_min(arg.first);
          proto.mutable_int32_range()->set_max(arg.second);
        } else if constexpr (std::is_same_v<T, ValueRange<int64_t>>) {
          proto.mutable_int64_range()->set_min(arg.first);
          proto.mutable_int64_range()->set_max(arg.second);
        } else if constexpr (std::is_same_v<T, ValueRange<float>>) {
          proto.mutable_float_range()->set_min(arg.first);
          proto.mutable_float_range()->set_max(arg.second);
        } else if constexpr (std::is_same_v<T, ValueRange<double>>) {
          proto.mutable_double_range()->set_min(arg.first);
          proto.mutable_double_range()->set_max(arg.second);
        }
      },
      value);
}

void from_proto(const tvsc::configuration::proto::RangedValue& proto, RangedValue& value) {
  static_assert(std::variant_size_v<RangedValue> == 4,
                "RangedValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<ValueRange<int32_t>, std::variant_alternative_t<0, RangedValue>>,
                "RangedValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<ValueRange<int64_t>, std::variant_alternative_t<1, RangedValue>>,
                "RangedValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<ValueRange<float>, std::variant_alternative_t<2, RangedValue>>,
                "RangedValue has been changed. Update this code block to match.");
  static_assert(std::is_same_v<ValueRange<double>, std::variant_alternative_t<3, RangedValue>>,
                "RangedValue has been changed. Update this code block to match.");

  if (proto.has_int32_range()) {
    ValueRange<int32_t> range{};
    from_proto(proto.int32_range(), range);
    value.emplace<ValueRange<int32_t>>(range);
  } else if (proto.has_int64_range()) {
    ValueRange<int64_t> range{};
    from_proto(proto.int64_range(), range);
    value.emplace<ValueRange<int64_t>>(range);
  } else if (proto.has_float_range()) {
    ValueRange<float> range{};
    from_proto(proto.float_range(), range);
    value.emplace<ValueRange<float>>(range);
  } else if (proto.has_double_range()) {
    ValueRange<double> range{};
    from_proto(proto.double_range(), range);
    value.emplace<ValueRange<double>>(range);
  }
}

void to_proto(const AllowedValues& value, tvsc::configuration::proto::AllowedValues& proto) {
  for (const auto& enumerated : value.enumerated_values()) {
    to_proto(enumerated, *proto.add_enumerated());
  }
  for (const auto& ranged : value.ranged_values()) {
    to_proto(ranged, *proto.add_ranged());
  }
}

void from_proto(const tvsc::configuration::proto::AllowedValues& proto, AllowedValues& values) {
  for (const auto& enumerated_proto : proto.enumerated()) {
    DiscreteValue value{};
    from_proto(enumerated_proto, value);
    values.add_enumerated_value(value);
  }
  for (const auto& ranged_proto : proto.ranged()) {
    RangedValue value{};
    from_proto(ranged_proto, value);
    values.add_ranged_value(value);
  }
}

void to_proto(const Function& function, tvsc::configuration::proto::Function& proto) {
  proto.set_function_id(function.id());
  proto.set_name(std::string{function.name()});

  to_proto(function.allowed_values(), *proto.mutable_allowed_values());
}

void from_proto(const tvsc::configuration::proto::Function& proto, Function& function) {
  function.set_id(proto.function_id());
  function.set_name(proto.name());
  AllowedValues allowed{};
  from_proto(proto.allowed_values(), allowed);
  function.set_allowed_values(allowed);
}

void to_proto(const Property& property, tvsc::configuration::proto::Property& proto) {
  proto.set_property_id(property.id());
  proto.set_name(std::string{property.name()});
  to_proto(property.value(), *proto.mutable_value());
}

void from_proto(const tvsc::configuration::proto::Property& proto, Property& property) {
  property.set_id(proto.property_id());
  property.set_name(proto.name());
  DiscreteValue value{};
  from_proto(proto.value(), value);
  property.set_value(value);
}

void to_proto(const Setting& setting, tvsc::configuration::proto::Setting& proto) {
  proto.set_function_id(setting.id());
  to_proto(setting.value(), *proto.mutable_value());
}

void from_proto(const tvsc::configuration::proto::Setting& proto, Setting& setting) {
  setting.set_id(proto.function_id());
  DiscreteValue value{};
  from_proto(proto.value(), value);
  setting.set_value(value);
}

void to_proto(const SystemDefinition& definition,
              tvsc::configuration::proto::SystemDefinition& proto) {
  proto.set_system_id(definition.id());
  proto.set_name(std::string{definition.name()});
  for (const auto& property : definition.properties()) {
    to_proto(property, *proto.add_properties());
  }

  for (const auto& function : definition.functions()) {
    to_proto(function, *proto.add_functions());
  }

  for (const auto& subsystem : definition.subsystems()) {
    to_proto(subsystem, *proto.add_subsystems());
  }
}

void from_proto(const tvsc::configuration::proto::SystemDefinition& proto,
                SystemDefinition& definition) {
  definition.set_id(proto.system_id());

  for (const auto& proto_property : proto.properties()) {
    Property property{};
    from_proto(proto_property, property);
    definition.add_property(property);
  }

  for (const auto& proto_function : proto.functions()) {
    Function function{};
    from_proto(proto_function, function);
    definition.add_function(function);
  }

  for (const auto& proto_subsystem : proto.subsystems()) {
    SystemDefinition system{};
    from_proto(proto_subsystem, system);
    definition.add_subsystem(system);
  }
}

void to_proto(const SystemConfiguration& configuration,
              tvsc::configuration::proto::SystemConfiguration& proto) {
  proto.set_system_id(configuration.id());

  for (const auto& setting : configuration.settings()) {
    to_proto(setting, *proto.add_settings());
  }

  for (const auto& subsystem : configuration.subsystems()) {
    to_proto(subsystem, *proto.add_subsystems());
  }
}

void from_proto(const tvsc::configuration::proto::SystemConfiguration& proto,
                SystemConfiguration& configuration) {
  configuration.set_id(proto.system_id());

  for (const auto& proto_setting : proto.settings()) {
    Setting setting{};
    from_proto(proto_setting, setting);
    configuration.add_setting(setting);
  }

  for (const auto& proto_subsystem : proto.subsystems()) {
    SystemConfiguration system{};
    from_proto(proto_subsystem, system);
    configuration.add_subsystem(system);
  }
}

namespace internal {

bool encode(const ValueRange<int32_t>& value, uint8_t* buf, size_t& size) {
  tvsc::configuration::proto::Int32Range proto{};
  to_proto(value, proto);

  const size_t proto_byte_size{proto.ByteSizeLong()};
  if (proto_byte_size <= size) {
    proto.SerializeWithCachedSizesToArray(buf);
    size = proto_byte_size;
    return true;
  } else {
    return false;
  }
}

bool decode(ValueRange<int32_t>& value, const uint8_t* buf, size_t size) {
  tvsc::configuration::proto::Int32Range proto{};

  bool result{proto.ParseFromArray(buf, size)};

  if (result) {
    from_proto(proto, value);
  }

  return result;
}

bool encode(const ValueRange<int64_t>& value, uint8_t* buf, size_t& size) {
  tvsc::configuration::proto::Int64Range proto{};
  to_proto(value, proto);

  const size_t proto_byte_size{proto.ByteSizeLong()};
  if (proto_byte_size <= size) {
    proto.SerializeWithCachedSizesToArray(buf);
    size = proto_byte_size;
    return true;
  } else {
    return false;
  }
}

bool decode(ValueRange<int64_t>& value, const uint8_t* buf, size_t size) {
  tvsc::configuration::proto::Int64Range proto{};

  bool result{proto.ParseFromArray(buf, size)};

  if (result) {
    from_proto(proto, value);
  }

  return result;
}

bool encode(const ValueRange<float>& value, uint8_t* buf, size_t& size) {
  tvsc::configuration::proto::FloatRange proto{};
  to_proto(value, proto);

  const size_t proto_byte_size{proto.ByteSizeLong()};
  if (proto_byte_size <= size) {
    proto.SerializeWithCachedSizesToArray(buf);
    size = proto_byte_size;
    return true;
  } else {
    return false;
  }
}

bool decode(ValueRange<float>& value, const uint8_t* buf, size_t size) {
  tvsc::configuration::proto::FloatRange proto{};

  bool result{proto.ParseFromArray(buf, size)};

  if (result) {
    from_proto(proto, value);
  }

  return result;
}

bool encode(const ValueRange<double>& value, uint8_t* buf, size_t& size) {
  tvsc::configuration::proto::DoubleRange proto{};
  to_proto(value, proto);

  const size_t proto_byte_size{proto.ByteSizeLong()};
  if (proto_byte_size <= size) {
    proto.SerializeWithCachedSizesToArray(buf);
    size = proto_byte_size;
    return true;
  } else {
    return false;
  }
}

bool decode(ValueRange<double>& value, const uint8_t* buf, size_t size) {
  tvsc::configuration::proto::DoubleRange proto{};

  bool result{proto.ParseFromArray(buf, size)};

  if (result) {
    from_proto(proto, value);
  }

  return result;
}

bool encode(const DiscreteValue& value, uint8_t* buf, size_t& size) {
  tvsc::configuration::proto::DiscreteValue proto{};
  to_proto(value, proto);

  const size_t proto_byte_size{proto.ByteSizeLong()};
  if (proto_byte_size <= size) {
    proto.SerializeWithCachedSizesToArray(buf);
    size = proto_byte_size;
    return true;
  } else {
    return false;
  }
}

bool decode(DiscreteValue& value, const uint8_t* buf, size_t size) {
  tvsc::configuration::proto::DiscreteValue proto{};

  bool result{proto.ParseFromArray(buf, size)};

  if (result) {
    from_proto(proto, value);
  }

  return result;
}

bool encode(const RangedValue& value, uint8_t* buf, size_t& size) {
  tvsc::configuration::proto::RangedValue proto{};
  to_proto(value, proto);

  const size_t proto_byte_size{proto.ByteSizeLong()};
  if (proto_byte_size <= size) {
    proto.SerializeWithCachedSizesToArray(buf);
    size = proto_byte_size;
    return true;
  } else {
    return false;
  }
}

bool decode(RangedValue& value, const uint8_t* buf, size_t size) {
  tvsc::configuration::proto::RangedValue proto{};

  bool result{proto.ParseFromArray(buf, size)};

  if (result) {
    from_proto(proto, value);
  }

  return result;
}

bool encode(const AllowedValues& value, uint8_t* buf, size_t& size) {
  tvsc::configuration::proto::AllowedValues proto{};
  to_proto(value, proto);

  const size_t proto_byte_size{proto.ByteSizeLong()};
  if (proto_byte_size <= size) {
    proto.SerializeWithCachedSizesToArray(buf);
    size = proto_byte_size;
    return true;
  } else {
    return false;
  }
}

bool decode(AllowedValues& value, const uint8_t* buf, size_t size) {
  tvsc::configuration::proto::AllowedValues proto{};

  bool result{proto.ParseFromArray(buf, size)};

  if (result) {
    from_proto(proto, value);
  }

  return result;
}

bool encode(const Function& fn, uint8_t* buf, size_t& size) {
  tvsc::configuration::proto::Function proto{};
  to_proto(fn, proto);

  const size_t proto_byte_size{proto.ByteSizeLong()};
  if (proto_byte_size <= size) {
    proto.SerializeWithCachedSizesToArray(buf);
    size = proto_byte_size;
    return true;
  } else {
    return false;
  }
}

bool decode(Function& fn, const uint8_t* buf, size_t size) {
  tvsc::configuration::proto::Function proto{};

  bool result{proto.ParseFromArray(buf, size)};

  if (result) {
    from_proto(proto, fn);
  }

  return result;
}

bool encode(const Property& value, uint8_t* buf, size_t& size) {
  tvsc::configuration::proto::Property proto{};
  to_proto(value, proto);

  const size_t proto_byte_size{proto.ByteSizeLong()};
  if (proto_byte_size <= size) {
    proto.SerializeWithCachedSizesToArray(buf);
    size = proto_byte_size;
    return true;
  } else {
    return false;
  }
}

bool decode(Property& value, const uint8_t* buf, size_t size) {
  tvsc::configuration::proto::Property proto{};

  bool result{proto.ParseFromArray(buf, size)};

  if (result) {
    from_proto(proto, value);
  }

  return result;
}

bool encode(const Setting& value, uint8_t* buf, size_t& size) {
  tvsc::configuration::proto::Setting proto{};
  to_proto(value, proto);

  const size_t proto_byte_size{proto.ByteSizeLong()};
  if (proto_byte_size <= size) {
    proto.SerializeWithCachedSizesToArray(buf);
    size = proto_byte_size;
    return true;
  } else {
    return false;
  }
}

bool decode(Setting& value, const uint8_t* buf, size_t size) {
  tvsc::configuration::proto::Setting proto{};

  bool result{proto.ParseFromArray(buf, size)};

  if (result) {
    from_proto(proto, value);
  }

  return result;
}

}  // namespace internal

bool encode(const SystemDefinition& definition, uint8_t* buf, size_t& size) {
  tvsc::configuration::proto::SystemDefinition proto{};
  to_proto(definition, proto);

  const size_t proto_byte_size{proto.ByteSizeLong()};
  if (proto_byte_size <= size) {
    proto.SerializeWithCachedSizesToArray(buf);
    size = proto_byte_size;
    return true;
  } else {
    return false;
  }
}

bool decode(SystemDefinition& definition, const uint8_t* buf, size_t size) {
  tvsc::configuration::proto::SystemDefinition proto{};

  bool result{proto.ParseFromArray(buf, size)};

  if (result) {
    from_proto(proto, definition);
  }

  return result;
}

bool encode(const SystemConfiguration& configuration, uint8_t* buf, size_t& size) {
  tvsc::configuration::proto::SystemConfiguration proto{};
  to_proto(configuration, proto);

  const size_t proto_byte_size{proto.ByteSizeLong()};
  if (proto_byte_size <= size) {
    proto.SerializeWithCachedSizesToArray(buf);
    size = proto_byte_size;
    return true;
  } else {
    return false;
  }
}

bool decode(SystemConfiguration& configuration, const uint8_t* buf, size_t size) {
  tvsc::configuration::proto::SystemConfiguration proto{};

  bool result{proto.ParseFromArray(buf, size)};

  if (result) {
    from_proto(proto, configuration);
  }

  return result;
}

}  // namespace tvsc::configuration
