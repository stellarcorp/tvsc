#include "hal2/config/type_accumulator.h"

#include "gtest/gtest.h"

namespace tvsc::hal2::config {

struct A {};
struct B {};
struct C {};
struct D {};
struct E {};

template <typename... Ts>
struct TypeHolder {};

TEST(TypeAccumulatorTest, InitialAccumulatorHasNoTypes) {
  using Acc = TypeAccumulator<>;

  static_assert(not Acc::contains<A>);
  static_assert(not Acc::contains<B>);
}

TEST(TypeAccumulatorTest, OneTypeAddedHasOneType) {
  using Acc = TypeAccumulator<>::Append<A>;

  static_assert(Acc::contains<A>);
  static_assert(not Acc::contains<B>);
}

TEST(TypeAccumulatorTest, SeveralTypesAddedHasThoseTypes) {
  using Acc = TypeAccumulator<>::Append<A>::Append<B>::Append<C>::Append<D>;

  static_assert(Acc::contains<A>);
  static_assert(Acc::contains<B>);
  static_assert(Acc::contains<C>);
  static_assert(Acc::contains<D>);
  static_assert(not Acc::contains<E>);
}

TEST(TypeAccumulatorTest, CanInstantiateTemplateParameterPackType) {
  using Acc = TypeAccumulator<A, B>::Append<C>;
  using Instantiated = Acc::Instantiate<TypeHolder>;

  static_assert(std::is_same_v<Instantiated, TypeHolder<A, B, C>>);
}

TEST(TypeAccumulatorTest, CanInstantiateTemplateParameterPackTypeWithOnePrefix) {
  using Acc = TypeAccumulator<B>::Append<C>;
  using Instantiated = Acc::InstantiateWithPrefix<TypeHolder, A>;

  static_assert(std::is_same_v<Instantiated, TypeHolder<A, B, C>>);
}

TEST(TypeAccumulatorTest, CanInstantiateTemplateParameterPackTypeWithMultiplePrefixes) {
  using Acc = TypeAccumulator<D, E>;
  using Instantiated = Acc::InstantiateWithPrefix<TypeHolder, A, B, C>;

  static_assert(std::is_same_v<Instantiated, TypeHolder<A, B, C, D, E>>);
}

}  // namespace tvsc::hal2::config
