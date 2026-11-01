#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <ranges>

#include "hal2/peripheral.h"
#include "hal2/config/peripheral_accessors.h"
#include "hal2/config/type_accumulator.h"
#include "hal2/config/vector_table.h"

namespace tvsc::hal2::config {

namespace internal {

/**
 * ConfiguredPeripheral bundles a static peripheral definition with its specific
 * compile-time configuration NTTP instance.
 */
template <typename Definition, auto Config>
struct ConfiguredPeripheral final {
  using definition = Definition;
  static constexpr auto config = Config;

  template <typename System, typename PeripheralRegistry>
  using accessor = typename definition::template accessor<System, PeripheralRegistry, config>;

  static constexpr PeripheralId id() noexcept { return definition::id(); }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    definition::template update_vector_table<NUM_ISRS>(table, config);
  }
};

/**
 * DependencyListResolver recursively flattens dependencies for peripherals in a system,
 * utilizing default configurations for automatically resolved upstream dependencies.
 */
template <typename PeripheralRegistry, typename Accumulator, typename Visiting, auto Dependencies,
          size_t Index>
struct DependencyListResolver {
  using type = Accumulator;
};

template <typename PeripheralRegistry, typename Accumulator, typename Visiting, auto Dependencies,
          size_t Index>
  requires(Index < Dependencies.size())
struct DependencyListResolver<PeripheralRegistry, Accumulator, Visiting, Dependencies, Index> {
  static constexpr auto identifier = Dependencies[Index];
  using CurrentDefinition =
      typename PeripheralRegistry::template definition<identifier.facility, identifier.index>;
  using CurrentConfigured =
      ConfiguredPeripheral<CurrentDefinition, typename CurrentDefinition::configuration{}>;

  // Trigger a compile-time error if this configuration is already an active ancestor.
  static_assert(not Visiting::template contains<CurrentConfigured>,
                "Circular dependency detected in peripheral configuration graph.");

  // Resolve sub-dependencies using the default configuration of the dependency.
  using AccumulatorWithSubDependencies = typename DependencyListResolver<
      PeripheralRegistry, Accumulator, typename Visiting::template Append<CurrentConfigured>,
      CurrentDefinition::dependencies(typename CurrentDefinition::configuration{}), 0>::type;

  using AccumulatorWithDependency =
      typename AccumulatorWithSubDependencies::template AppendUnique<CurrentConfigured>;

  using type = typename DependencyListResolver<PeripheralRegistry, AccumulatorWithDependency,
                                               Visiting, Dependencies, Index + 1>::type;
};

}  // namespace internal

template <typename PeripheralRegistry, typename... ConfiguredPeripherals>
class System final
    : public ConfiguredPeripherals::template accessor<
          System<PeripheralRegistry, ConfiguredPeripherals...>, PeripheralRegistry>... {
 private:
  template <typename, typename>
  friend class SystemBuilder;

  constexpr System() noexcept = default;

 public:
  // All public methods come from inherited classes.
};

template <typename PeripheralRegistry, typename Accumulator = TypeAccumulator<>>
class SystemBuilder;

template <typename PeripheralRegistry, typename... ConfiguredPeripherals>
class SystemBuilder<PeripheralRegistry, TypeAccumulator<ConfiguredPeripherals...>> final {
 public:
  constexpr SystemBuilder() noexcept = default;

  template <Facility FACILITY, size_t INDEX,
            auto CONFIG =
                typename PeripheralRegistry::template definition<FACILITY, INDEX>::configuration{}>
  [[nodiscard]] consteval auto add() noexcept {
    using TargetDefinition = typename PeripheralRegistry::template definition<FACILITY, INDEX>;

    static_assert(TargetDefinition::is_valid(CONFIG),
                  "Invalid peripheral configuration parameters.");

    using TargetConfigured = internal::ConfiguredPeripheral<TargetDefinition, CONFIG>;

    // Resolve all transitive dependencies required by the target configuration using
    // the configuration's custom or default parameters.
    using AccumulatorWithDependencies =
        typename internal::DependencyListResolver<PeripheralRegistry,                         //
                                                  TypeAccumulator<ConfiguredPeripherals...>,  //
                                                  TypeAccumulator<TargetConfigured>,          //
                                                  TargetDefinition::dependencies(CONFIG), 0>::type;

    using NextAccumulator =
        typename AccumulatorWithDependencies::template AppendUnique<TargetConfigured>;

    return SystemBuilder<PeripheralRegistry, NextAccumulator>{};
  }

  template <size_t NUM_ISRS = 127>
  [[nodiscard]] consteval auto create_vector_table() const noexcept {
    VectorTable<NUM_ISRS> vector_table{};
    (ConfiguredPeripherals::template update_vector_table<NUM_ISRS>(vector_table), ...);
    return vector_table;
  }

  constexpr auto system() const noexcept {
    using ConcreteSystem = TypeAccumulator<
        ConfiguredPeripherals...>::template InstantiateWithPrefix<System, PeripheralRegistry>;
    return ConcreteSystem{};
  }
};

}  // namespace tvsc::hal2::config
