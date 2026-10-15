#pragma once

#include <concepts>

#include "hal/config/facilities.h"
#include "hal/config/facility_accessors.h"
#include "hal/config/vector_table.h"

namespace tvsc::hal::config {

template <Facility F, size_t INDEX>
struct FacilityConfiguration;

template <typename T>
concept ValidFacilityConfiguration = requires {
  // FacilityConfiguration should expose the type of the accessor they expose.
  typename T::accessor;

  { T::facility() } -> std::same_as<Facility>;
  { T::index() } -> std::same_as<size_t>;

  requires ValidFacilityAccessor<typename T::accessor>;

  requires T::accessor::facility() == T::facility();
  requires T::accessor::index() == T::index();

  // Enforce that FacilityConfigurations provide:
  // template <size_t NUM_ISRS>
  // static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table) noexcept;
  requires requires(VectorTable<16> table) {
    { T::template update_vector_table<16>(table) } noexcept -> std::same_as<void>;
  };
};

template <typename... FacilityAccessors>
struct System final : FacilityAccessors... {
  // All of the System's methods come from the FacilityAccessors.
};

template <size_t NUM_ISRS, typename... FacilityConfigurations>
struct SystemBuilder final : FacilityConfigurations... {
  template <ValidFacilityConfiguration Configuration>
  [[nodiscard]] consteval auto add() noexcept {
    return SystemBuilder<NUM_ISRS, FacilityConfigurations..., Configuration>{};
  }

  [[nodiscard]] consteval auto create_vector_table() const noexcept {
    VectorTable<NUM_ISRS> vector_table{};
    (FacilityConfigurations::template update_vector_table<NUM_ISRS>(vector_table), ...);
    return vector_table;
  }

  [[nodiscard]] consteval auto system() const noexcept {
    return System<typename FacilityConfigurations::accessor...>{};
  }
};

}  // namespace tvsc::hal::config
