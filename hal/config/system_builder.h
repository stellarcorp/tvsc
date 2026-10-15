#pragma once

#include <concepts>

#include "hal/config/facility.h"
#include "hal/config/peripheral_accessors.h"
#include "hal/config/vector_table.h"

namespace tvsc::hal::config {

template <typename T>
concept ValidPeripheralConfiguration = requires {
  // PeripheralConfiguration should expose the type of the accessor they expose.
  typename T::accessor;

  { T::facility() } -> std::same_as<Facility>;
  { T::index() } -> std::same_as<size_t>;

  requires ValidPeripheralAccessor<typename T::accessor>;

  requires T::accessor::facility() == T::facility();
  requires T::accessor::index() == T::index();

  // Enforce that PeripheralConfigurations provide:
  // template <size_t NUM_ISRS>
  // static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table) noexcept;
  requires requires(VectorTable<16> table) {
    { T::template update_vector_table<16>(table) } noexcept -> std::same_as<void>;
  };
};

template <typename... PeripheralAccessors>
struct System final : PeripheralAccessors... {
  // All of the System's methods come from its superclasses, the PeripheralAccessors.
};

template <size_t NUM_ISRS, typename... PeripheralConfigurations>
struct SystemBuilder final : PeripheralConfigurations... {
  template <ValidPeripheralConfiguration Configuration>
  [[nodiscard]] consteval auto add() noexcept {
    return SystemBuilder<NUM_ISRS, PeripheralConfigurations..., Configuration>{};
  }

  [[nodiscard]] consteval auto create_vector_table() const noexcept {
    VectorTable<NUM_ISRS> vector_table{};
    (PeripheralConfigurations::template update_vector_table<NUM_ISRS>(vector_table), ...);
    return vector_table;
  }

  [[nodiscard]] consteval auto system() const noexcept {
    return System<typename PeripheralConfigurations::accessor...>{};
  }
};

}  // namespace tvsc::hal::config
