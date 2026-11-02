#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <ranges>

#include "hal2/config/device_definitions.h"
#include "hal2/config/type_accumulator.h"
#include "hal2/config/vector_table.h"
#include "hal2/device.h"

namespace tvsc::hal2::config {

namespace internal {

/**
 * ConfiguredDevice combines a device definition with a specific configuration instance.
 */
template <typename Definition, auto config>
struct ConfiguredDevice final {
  using definition = Definition;
  static constexpr auto CONFIG{config};

  template <typename System, typename DeviceRegistry>
  using accessor = typename definition::template accessor<System, DeviceRegistry, CONFIG>;

  static constexpr DeviceId id() noexcept { return definition::id(); }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    definition::template update_vector_table<NUM_ISRS>(table, CONFIG);
  }
};

/**
 * DependencyListResolver recursively flattens dependencies for devices in a system,
 * utilizing default configurations for automatically resolved upstream dependencies.
 */
template <typename DeviceRegistry, typename Accumulator, typename Visiting, auto Dependencies,
          size_t Index>
struct DependencyListResolver {
  using type = Accumulator;
};

template <typename DeviceRegistry, typename Accumulator, typename Visiting, auto Dependencies,
          size_t Index>
  requires(Index < Dependencies.size())
struct DependencyListResolver<DeviceRegistry, Accumulator, Visiting, Dependencies, Index> {
  static constexpr auto identifier = Dependencies[Index];
  using CurrentDefinition =
      typename DeviceRegistry::template definition<identifier.facility, identifier.index>;
  using CurrentConfigured =
      ConfiguredDevice<CurrentDefinition, typename CurrentDefinition::configuration{}>;

  // Trigger a compile-time error if this configuration is already an active ancestor.
  static_assert(not Visiting::template contains<CurrentConfigured>,
                "Circular dependency detected in device configuration graph.");

  // Resolve sub-dependencies using the default configuration of the dependency.
  using AccumulatorWithSubDependencies = typename DependencyListResolver<
      DeviceRegistry, Accumulator, typename Visiting::template Append<CurrentConfigured>,
      CurrentDefinition::dependencies(typename CurrentDefinition::configuration{}), 0>::type;

  using AccumulatorWithDependency =
      typename AccumulatorWithSubDependencies::template AppendUnique<CurrentConfigured>;

  using type = typename DependencyListResolver<DeviceRegistry, AccumulatorWithDependency, Visiting,
                                               Dependencies, Index + 1>::type;
};

}  // namespace internal

template <typename DeviceRegistry, typename... ConfiguredDevices>
class System final
    : public ConfiguredDevices::template accessor<System<DeviceRegistry, ConfiguredDevices...>,
                                                  DeviceRegistry>... {
 private:
  template <typename, typename>
  friend class SystemBuilder;

  constexpr System() noexcept = default;

 public:
  // All public methods come from inherited classes.
};

template <typename DeviceRegistry, typename Accumulator = TypeAccumulator<>>
class SystemBuilder;

template <typename DeviceRegistry, typename... ConfiguredDevices>
class SystemBuilder<DeviceRegistry, TypeAccumulator<ConfiguredDevices...>> final {
 public:
  constexpr SystemBuilder() noexcept = default;

  template <
      Facility FACILITY, size_t INDEX,
      typename DeviceRegistry::template definition<FACILITY, INDEX>::configuration CONFIG = {}>
  [[nodiscard]] consteval auto add() noexcept {
    using TargetDefinition = typename DeviceRegistry::template definition<FACILITY, INDEX>;

    static_assert(ValidDeviceDefinition<TargetDefinition, DeviceRegistry>,
                  "Invalid device definition");
    static_assert(TargetDefinition::is_valid(CONFIG), "Invalid device configuration parameters.");
    static_assert(
        (!std::is_same_v<typename ConfiguredDevices::definition, TargetDefinition> && ...),
        "This device definition has already been added to the system. Each device definition "
        "(Facility and index pair) must be unique in the system. Either this device was explicitly "
        "added previously, or it was brought in earlier as a dependency by another device. If the "
        "latter, adding this device definition earlier in the system builder calls will resolve "
        "this issue, but you should double-check the configuration of this device.");

    using TargetConfigured = internal::ConfiguredDevice<TargetDefinition, CONFIG>;

    // Resolve all transitive dependencies required by the target configuration using
    // the configuration's custom or default parameters.
    using AccumulatorWithDependencies =
        typename internal::DependencyListResolver<DeviceRegistry,                         //
                                                  TypeAccumulator<ConfiguredDevices...>,  //
                                                  TypeAccumulator<TargetConfigured>,      //
                                                  TargetDefinition::dependencies(CONFIG), 0>::type;

    using NextAccumulator =
        typename AccumulatorWithDependencies::template AppendUnique<TargetConfigured>;

    return SystemBuilder<DeviceRegistry, NextAccumulator>{};
  }

  template <size_t NUM_ISRS = 127>
  [[nodiscard]] consteval auto create_vector_table() const noexcept {
    VectorTable<NUM_ISRS> vector_table{};
    (ConfiguredDevices::template update_vector_table<NUM_ISRS>(vector_table), ...);
    return vector_table;
  }

  constexpr auto system() const noexcept {
    using ConcreteSystem =
        TypeAccumulator<ConfiguredDevices...>::template InstantiateWithPrefix<System,
                                                                              DeviceRegistry>;
    return ConcreteSystem{};
  }
};

}  // namespace tvsc::hal2::config
