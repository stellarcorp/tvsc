#pragma once

namespace tvsc::comms::radio {

class Setting final {
 private:
  Function function_;
  Value value_;
  AllowedValues allowed_values_;

 public:
  Setting(Function function, Value value, AllowedValues&& allowed_values)
      : function_(function), value_(value), allowed_values_(std::move(allowed_values)) {}

  ~Setting() = default;

  Setting(const Setting& rhs) = default;
  Setting(Setting&& rhs) = default;
  Setting& operator=(const Setting& rhs) = default;
  Setting& operator=(Setting&& rhs) = default;

  constexpr std::string_view name() const;
  constexpr std::string_view description() const;

  Function function() const { return function_; }

  const AllowedValues& allowed_values() const { return allowed_values_; }

  Value value() const { return value_; }
  void set_value(const Value& value) { value_ = value; }
};

class RadioSettings final {
 private:
  std::vector<Setting> settings_{};

 public:
  RadioSettings() = default;
  ~RadioSettings() = default;
  RadioSettings(const RadioSettings& rhs) = default;
  RadioSettings(RadioSettings&& rhs) = default;
  RadioSettings& operator=(const RadioSettings& rhs) = default;
  RadioSettings& operator=(RadioSettings&& rhs) = default;

  const std::vector<Setting>& settings() const { return settings_; }
};

}  // namespace tvsc::comms::radio
