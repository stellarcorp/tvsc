#pragma once

#include <concepts>

#include "hal2/config/facility.h"
#include "hal2/config/peripheral_accessors.h"
#include "hal2/config/vector_table.h"

namespace tvsc::hal2::config {

template <typename T>
concept ValidPeripheralConfiguration =
    requires {
      // PeripheralConfiguration should expose an accessor type templated on board type.
      typename T::template accessor<int>;
      { T::facility() } -> std::same_as<Facility>;
      { T::index() } -> std::same_as<size_t>;
    } and  //
    ValidPeripheralAccessor<typename T::template accessor<int>, T::facility(), T::index()> and
    requires(VectorTable<16> table) {
      // Enforce vector table updater requirement.
      { T::template update_vector_table<16>(table) } noexcept -> std::same_as<void>;
    };

template <typename... PeripheralConfigurations>
struct SystemBuilder;

template <typename Board, typename... PeripheralConfigurations>
class System final : public PeripheralConfigurations::template accessor<
                         System<Board, PeripheralConfigurations...>>... {
 private:
  Board board_;

  template <typename...>
  friend struct SystemBuilder;

  constexpr System(Board board) : board_(std::move(board)) {}

 public:
  [[nodiscard]] constexpr const Board& board() const noexcept { return board_; }
  [[nodiscard]] constexpr Board& board() noexcept { return board_; }
};

template <typename... PeripheralConfigurations>
struct SystemBuilder final : PeripheralConfigurations... {
  template <ValidPeripheralConfiguration Configuration>
  // Require that a configuration type may be only added once.
    requires(!(std::is_same_v<Configuration, PeripheralConfigurations> or ...))
  [[nodiscard]] consteval auto add() noexcept {
    return SystemBuilder<PeripheralConfigurations..., Configuration>{};
  }

  template <size_t NUM_ISRS>
  [[nodiscard]] consteval auto create_vector_table() const noexcept {
    VectorTable<NUM_ISRS> vector_table{};
    (PeripheralConfigurations::template update_vector_table<NUM_ISRS>(vector_table), ...);
    return vector_table;
  }

  template <typename Board>
  consteval auto system(Board board) const noexcept {
    return System<Board, PeripheralConfigurations...>{std::move(board)};
  }
};

}  // namespace tvsc::hal2::config
