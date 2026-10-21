#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <ranges>

#include "hal2/config/facility.h"
#include "hal2/config/peripheral_accessors.h"
#include "hal2/config/type_accumulator.h"
#include "hal2/config/vector_table.h"

namespace tvsc::hal2::config {

template <typename T, typename PeripheralRegistry>
concept ValidPeripheralConfiguration =
    requires {
      // PeripheralConfiguration should expose an accessor type templated on full System type.
      typename T::template accessor<int, PeripheralRegistry>;
      { T::id() } -> std::same_as<PeripheralId>;
    } and  //
    ValidPeripheralAccessor<typename T::template accessor<int, PeripheralRegistry>, T::id()> and
    requires(VectorTable<16>& table) {
      { T::update_vector_table(table) } noexcept -> std::same_as<void>;
    } and  //
    requires {
      { T::dependencies() } noexcept -> std::ranges::range;
      std::is_same_v<std::ranges::range_value_t<decltype(T::dependencies())>, PeripheralId>;
    };

template <typename PeripheralRegistry, typename Accumulator = TypeAccumulator<>>
class SystemBuilder;

template <typename PeripheralRegistry, typename... PeripheralConfigurations>
class System final
    : public PeripheralConfigurations::template accessor<
          System<PeripheralRegistry, PeripheralConfigurations...>, PeripheralRegistry>... {
 private:
  template <typename, typename>
  friend class SystemBuilder;

  constexpr System() noexcept = default;

 public:
  // All public methods come from inherited classes.
};

namespace internal {

/**
 * DependencyListResolver recursively flattens dependencies for peripherals in a system.
 */
// Terminate dependency resolution once all of the items in the array have been processed. This
// triggers when the Index >= Dependencies.size(), since the iteration form below requires Index <
// Dependencies.size().
template <typename PeripheralRegistry, typename Accumulator, typename Visiting, auto Dependencies,
          size_t Index>
struct DependencyListResolver {
  using type = Accumulator;
};

// Recursively walk the dependency array to resolve peripherals by id.
template <typename PeripheralRegistry, typename Accumulator, typename Visiting, auto Dependencies,
          size_t Index>
  requires(Index < Dependencies.size())
struct DependencyListResolver<PeripheralRegistry, Accumulator, Visiting, Dependencies, Index> {
  static constexpr auto identifier = Dependencies[Index];
  using CurrentConfiguration =
      typename PeripheralRegistry::template configuration<identifier.facility, identifier.index>;

  // Trigger a compile-time error if this configuration is already an active ancestor.
  static_assert(not Visiting::template contains<CurrentConfiguration>,
                "Circular dependency detected in peripheral configuration graph.");

  // Resolve any sub-dependencies of this specific peripheral before appending it.
  using AccumulatorWithSubDependencies =
      typename DependencyListResolver<PeripheralRegistry, Accumulator,
                                      typename Visiting::template Append<CurrentConfiguration>,
                                      CurrentConfiguration::dependencies(), 0>::type;

  using AccumulatorWithDependency =
      typename AccumulatorWithSubDependencies::template AppendUnique<CurrentConfiguration>;

  // Advance to the next dependency.
  using type = typename DependencyListResolver<PeripheralRegistry, AccumulatorWithDependency,
                                               Visiting, Dependencies, Index + 1>::type;
};

}  // namespace internal

template <typename PeripheralRegistry, typename... PeripheralConfigurations>
class SystemBuilder<PeripheralRegistry, TypeAccumulator<PeripheralConfigurations...>> final {
 public:
  constexpr SystemBuilder() noexcept = default;

  template <Facility FACILITY, size_t INDEX>
  consteval auto add() noexcept {
    using TargetConfiguration =
        typename PeripheralRegistry::template configuration<FACILITY, INDEX>;

    // Resolve all transitive dependencies required by the target configuration before appending it
    // to the system.
    using AccumulatorWithDependencies =
        typename internal::DependencyListResolver<PeripheralRegistry,                            //
                                                  TypeAccumulator<PeripheralConfigurations...>,  //
                                                  TypeAccumulator<TargetConfiguration>,          //
                                                  TargetConfiguration::dependencies(), 0>::type;

    using NextAccumulator =
        typename AccumulatorWithDependencies::template AppendUnique<TargetConfiguration>;

    return SystemBuilder<PeripheralRegistry, NextAccumulator>{};
  }

  template <size_t NUM_ISRS>
  [[nodiscard]] consteval auto create_vector_table() const noexcept {
    VectorTable<NUM_ISRS> vector_table{};
    (PeripheralConfigurations::template update_vector_table<NUM_ISRS>(vector_table), ...);
    return vector_table;
  }

  constexpr auto system() const noexcept {
    using ConcreteSystem = TypeAccumulator<
        PeripheralConfigurations...>::template InstantiateWithPrefix<System, PeripheralRegistry>;
    return ConcreteSystem{};
  }
};

}  // namespace tvsc::hal2::config
