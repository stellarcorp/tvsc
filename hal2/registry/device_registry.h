#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <ranges>

#include "hal2/registry/device_definitions.h"
#include "hal2/registry/type_accumulator.h"
#include "hal2/registry/vector_table.h"
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

  template <typename DeviceRegistry, typename Board>
  using accessor = typename definition::template accessor<DeviceRegistry, Board, CONFIG>;

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
template <typename Board, typename Accumulator, typename Visiting, auto Dependencies, size_t Index>
struct DependencyListResolver {
  using type = Accumulator;
};

template <typename Board, typename Accumulator, typename Visiting, auto Dependencies, size_t Index>
  requires(Index < Dependencies.size())
struct DependencyListResolver<Board, Accumulator, Visiting, Dependencies, Index> {
  static constexpr auto identifier = Dependencies[Index];
  using CurrentDefinition = typename Board::template devices<identifier.facility, identifier.index>;
  using CurrentConfigured =
      ConfiguredDevice<CurrentDefinition, typename CurrentDefinition::configuration{}>;

  // Trigger a compile-time error if this configuration is already an active ancestor.
  static_assert(not Visiting::template contains<CurrentConfigured>,
                "Circular dependency detected in device configuration graph.");

  // Resolve sub-dependencies using the default configuration of the dependency.
  using AccumulatorWithSubDependencies = typename DependencyListResolver<
      Board, Accumulator, typename Visiting::template Append<CurrentConfigured>,
      CurrentDefinition::dependencies(typename CurrentDefinition::configuration{}), 0>::type;

  using AccumulatorWithDependency =
      typename AccumulatorWithSubDependencies::template AppendUnique<CurrentConfigured>;

  using type = typename DependencyListResolver<Board, AccumulatorWithDependency, Visiting,
                                               Dependencies, Index + 1>::type;
};

}  // namespace internal

template <typename Board, typename... ConfiguredDevices>
class DeviceRegistry final
    : public ConfiguredDevices::template accessor<DeviceRegistry<Board, ConfiguredDevices...>,
                                                  Board>... {
 private:
  template <typename, typename>
  friend class DeviceRegistryBuilder;

  constexpr DeviceRegistry() noexcept = default;

 public:
  template <DeviceId DEVICE>
  [[nodiscard]] constexpr auto& as_accessor() noexcept {
    using accessor_type =
        typename Board::template devices<DEVICE.facility, DEVICE.index>::template accessor<
            DeviceRegistry, Board,
            typename Board::template devices<DEVICE.facility, DEVICE.index>::configuration{}>;
    return static_cast<accessor_type&>(*this);
  }

  template <DeviceId DEVICE>
  [[nodiscard]] constexpr auto& get_device() noexcept {
    return as_accessor<DEVICE>().get();
  }
};

// TODO(james): Extract Board as a concept and rephrase the first type parameter below to
// "Board Board".
template <typename Board, typename Accumulator = TypeAccumulator<>>
class DeviceRegistryBuilder;

template <typename Board, typename... ConfiguredDevices>
class DeviceRegistryBuilder<Board, TypeAccumulator<ConfiguredDevices...>> final {
 public:
  constexpr DeviceRegistryBuilder() noexcept = default;

  template <Facility FACILITY, size_t INDEX,
            typename Board::template devices<FACILITY, INDEX>::configuration CONFIG = {}>
  [[nodiscard]] consteval auto add() noexcept {
    using TargetDefinition = typename Board::template devices<FACILITY, INDEX>;

    static_assert(ValidDeviceDefinition<TargetDefinition, Board>, "Invalid device definition");
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
        typename internal::DependencyListResolver<Board,                                  //
                                                  TypeAccumulator<ConfiguredDevices...>,  //
                                                  TypeAccumulator<TargetConfigured>,      //
                                                  TargetDefinition::dependencies(CONFIG), 0>::type;

    using NextAccumulator =
        typename AccumulatorWithDependencies::template AppendUnique<TargetConfigured>;

    return DeviceRegistryBuilder<Board, NextAccumulator>{};
  }

  template <size_t NUM_ISRS = 127>
  [[nodiscard]] consteval auto create_vector_table() const noexcept {
    VectorTable<NUM_ISRS> vector_table{};
    (ConfiguredDevices::template update_vector_table<NUM_ISRS>(vector_table), ...);
    return vector_table;
  }

  constexpr auto registry() const noexcept {
    using ConcreteDeviceRegistry =
        TypeAccumulator<ConfiguredDevices...>::template InstantiateWithPrefix<DeviceRegistry,
                                                                              Board>;
    return ConcreteDeviceRegistry{};
  }
};

}  // namespace tvsc::hal2::config
