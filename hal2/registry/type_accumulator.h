#pragma once

#include <type_traits>

namespace tvsc::hal2::config {

template <typename... Types>
struct TypeAccumulator final {
  template <typename Type>
  static constexpr bool contains{(std::is_same_v<Type, Types> or ...)};

  template <typename Type>
  using Append = TypeAccumulator<Types..., Type>;

  template <typename... OtherTypes>
  using Concat = TypeAccumulator<Types..., OtherTypes...>;

  template <typename Type>
  using AppendUnique = std::conditional_t<contains<Type>,                  //
                                          TypeAccumulator<Types...>,       //
                                          TypeAccumulator<Types..., Type>  //
                                          >;

  // Helper types to instantiate a target class template based on the accumulated types.
  template <template <typename...> class Target>
  using Instantiate = Target<Types...>;

  template <template <typename...> class Target, typename... Prefix>
  using InstantiateWithPrefix = Target<Prefix..., Types...>;
};

}  // namespace tvsc::hal2::config
