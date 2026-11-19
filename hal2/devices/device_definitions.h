#pragma once

#include <cstddef>

#include "hal2/devices/device.h"
#include "hal2/devices/device_accessors.h"
#include "hal2/devices/vector_table.h"

namespace tvsc::hal2::devices {

// TODO(james): Add constraint to restrict the DeviceRegistry type parameter to the DeviceRegistry
// type from a DeviceRegistryBuilder.
template <typename DeviceRegistry, typename Board, DeviceId Id>
using accessor_type = typename Board::template devices<Id.facility, Id.index>::template accessor<
    DeviceRegistry, Board,
    typename Board::template devices<Id.facility, Id.index>::configuration{}>;

// TODO(james): Add constraint to restrict the DeviceRegistry type parameter to the DeviceRegistry
// type from a DeviceRegistryBuilder.
template <DeviceId ID, typename DeviceRegistry>
[[nodiscard]] constexpr auto& get_device(auto& system) noexcept {
  return static_cast<DeviceRegistry&>(system).template get_device<ID>();
}

// TODO(james): Add constraint to restrict the DeviceRegistry type parameter to the DeviceRegistry
// type from a DeviceRegistryBuilder.
template <DeviceId ID, typename DeviceRegistry>
[[nodiscard]] constexpr auto& get_accessor(auto& system) noexcept {
  return static_cast<DeviceRegistry&>(system).template as_accessor<ID>();
}

/**
 * Basic device setup when the device does not have any configuration parameters. Note that
 * the methods below can be overridden in child types.
 */
struct UnconfigurableDevice {
  struct Unconfigurable final {};
  using configuration = Unconfigurable;

  static constexpr bool is_valid(const configuration&) noexcept { return true; }

  static constexpr auto dependencies(const configuration&) noexcept {
    // Almost every device requires baseline RCC functionality. The most important exception is
    // the RCC device itself.
    return std::array{DeviceId{Facility::RCC_FACILITY, 0}};
  }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(VectorTable<NUM_ISRS>&, const configuration&) noexcept {
    // No ISRs required by default.
  }
};

template <typename T, typename Board>
concept ValidDeviceDefinition =
    requires {
      typename T::configuration;
      { T::name() } noexcept -> std::same_as<const char*>;
      { T::id() } noexcept -> std::same_as<DeviceId>;
    } and
    requires(typename T::configuration config) {
      { T::is_valid(config) } noexcept -> std::same_as<bool>;
      { T::dependencies(config) } noexcept -> std::ranges::range;
      requires std::is_same_v<std::ranges::range_value_t<decltype(T::dependencies(config))>,
                              DeviceId>;
    } and
    requires(VectorTable<16>& table, typename T::configuration config) {
      { T::update_vector_table(table, config) } noexcept -> std::same_as<void>;
    } and
    requires {
      // Verify that the accessor template instantiates cleanly using a default configuration NTTP
      // value.
      typename T::template accessor<int, Board, typename T::configuration{}>;
    } and
    ValidDeviceAccessor<typename T::template accessor<int, Board, typename T::configuration{}>,
                        T::id()>;

}  // namespace tvsc::hal2::devices
