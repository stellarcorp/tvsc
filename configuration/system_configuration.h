#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "configuration/allowed_values.h"
#include "configuration/system_definition.h"

namespace tvsc::configuration {

class Setting final {
 private:
  FunctionId id_;
  DiscreteValue value_;

 public:
  Setting(FunctionId id, const DiscreteValue& value) : id_(id), value_(value) {}

  operator FunctionId() const { return id(); }
  FunctionId id() const { return id_; }

  const DiscreteValue& value() const { return value_; }
};

class SystemConfiguration final {
 private:
  SystemId id_;
  std::vector<SystemConfiguration> subsystems_{};
  std::vector<Setting> settings_{};

 public:
  SystemConfiguration(SystemId id) : id_(id) {}

  SystemConfiguration(SystemId id, std::initializer_list<SystemConfiguration> subsystems)
      : id_(id), subsystems_(subsystems.begin(), subsystems.end()) {
    std::sort(subsystems_.begin(), subsystems_.end());
  }

  SystemConfiguration(SystemId id, std::initializer_list<Setting> settings)
      : id_(id), settings_(settings.begin(), settings.end()) {
    std::sort(settings_.begin(), settings_.end());
  }

  SystemConfiguration(SystemId id, std::initializer_list<SystemConfiguration> subsystems,
                      std::initializer_list<Setting> settings)
      : id_(id),
        subsystems_(subsystems.begin(), subsystems.end()),
        settings_(settings.begin(), settings.end()) {
    std::sort(subsystems_.begin(), subsystems_.end());
    std::sort(settings_.begin(), settings_.end());
  }

  operator SystemId() const { return id(); }
  SystemId id() const { return id_; }

  const std::vector<SystemConfiguration>& subsystems() const { return subsystems_; }
  const std::vector<Setting>& settings() const { return settings_; }
};

bool is_valid_configuration(const SystemDefinition& system,
                            const SystemConfiguration& configuration);

std::string to_string(const Setting& setting);
std::string to_string(const SystemConfiguration& configuration);

}  // namespace tvsc::configuration
