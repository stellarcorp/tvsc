#pragma once

#include <cstddef>

#include "hal2/config/peripheral_accessors.h"
#include "hal2/config/vector_table.h"
#include "hal2/device.h"

namespace tvsc::hal2::config {

template <typename System, typename DeviceRegistry, DeviceId Id>
using accessor_type =
    typename DeviceRegistry::template definition<Id.facility, Id.index>::template accessor<
        System, DeviceRegistry,
        typename DeviceRegistry::template definition<Id.facility, Id.index>::configuration{}>;

/**
 * Basic peripheral setup when the peripheral does not have any configuration parameters.
 */
struct UnconfigurableDevice {
  struct Unconfigurable final {};
  using configuration = Unconfigurable;

  static constexpr bool is_valid(const configuration&) noexcept { return true; }

  static constexpr auto dependencies(const configuration&) noexcept {
    // Almost every peripheral requires baseline RCC functionality. The most important exception is
    // the RCC peripheral itself.
    return std::array{DeviceId{Facility::RCC_FACILITY, 0}};
  }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>&,
                                            const configuration&) noexcept {
    // No ISRs required by default.
  }
};

template <typename T, typename DeviceRegistry>
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
      typename T::template accessor<int, DeviceRegistry, typename T::configuration{}>;
    } and
    ValidDeviceAccessor<
        typename T::template accessor<int, DeviceRegistry, typename T::configuration{}>,
        T::id()>;

}  // namespace tvsc::hal2::config
