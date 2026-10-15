#include "hal/config/system_builder.h"

#include <cstddef>

#include "gtest/gtest.h"
#include "hal/config/facility.h"
#include "hal/config/peripheral_accessors.h"
#include "hal/config/vector_table.h"

// Symbols provided by startup assembly and the linker. These empty versions are provided here to
// make the tests work.
extern "C" {
char _estack[0];
void Reset_Handler(void) {}
}

namespace tvsc::hal::config {

struct RccAccessor {
  static constexpr Facility facility() noexcept { return Facility::RCC; }
  static constexpr size_t index() noexcept { return 0; }

  int modifiable_value{42};
  bool initialized_at_compile_time{std::is_constant_evaluated()};

  int rcc() {
    // Production accessors would return an actual peripheral here, not an int.
    return 100;
  }
};
static_assert(ValidPeripheralAccessor<RccAccessor>);

struct SystickAccessor {
  static constexpr Facility facility() noexcept { return Facility::SYSTICK; }
  static constexpr size_t index() noexcept { return 0; }
  int systick() { return 200; }
};
static_assert(ValidPeripheralAccessor<SystickAccessor>);

template <size_t INDEX>
struct I2cAccessor;

template <>
struct I2cAccessor<0> {
  static constexpr Facility facility() noexcept { return Facility::I2C; }
  static constexpr size_t index() noexcept { return 0; }
  constexpr int i2c0() const noexcept { return 300; }
};
static_assert(ValidPeripheralAccessor<I2cAccessor<0>>);

template <>
struct I2cAccessor<1> {
  static constexpr Facility facility() noexcept { return Facility::I2C; }
  static constexpr size_t index() noexcept { return 1; }
  constexpr int i2c1() const noexcept { return 400; }
};
static_assert(ValidPeripheralAccessor<I2cAccessor<1>>);

inline void rcc_isr() noexcept {}
inline void systick_isr() noexcept {}
inline void i2c0_ev_isr() noexcept {}
inline void i2c0_er_isr() noexcept {}
inline void i2c1_ev_isr() noexcept {}
inline void i2c1_er_isr() noexcept {}

static constexpr size_t NUM_ISRS{14};
static constexpr size_t RCC_IRQn{12};
// Normally, the ARM header files list SysTick_IRQn as -1. The actual configuration will be
// responsible for translating that -1 value into a valid index.
static constexpr size_t SYSTICK_IRQn{2};
static constexpr size_t I2C0_EV_IRQn{4};
static constexpr size_t I2C0_ER_IRQn{6};
static constexpr size_t I2C1_EV_IRQn{5};
static constexpr size_t I2C1_ER_IRQn{7};

struct RccConfiguration {
  using accessor = RccAccessor;

  static constexpr Facility facility() noexcept { return Facility::RCC; }
  static constexpr size_t index() noexcept { return 0; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table) noexcept {
    table.isrs[RCC_IRQn] = &rcc_isr;
  }
};
static_assert(ValidPeripheralConfiguration<RccConfiguration>);

struct SystickConfiguration {
  using accessor = SystickAccessor;

  static constexpr Facility facility() noexcept { return Facility::SYSTICK; }
  static constexpr size_t index() noexcept { return 0; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table) noexcept {
    table.isrs[SYSTICK_IRQn] = &systick_isr;
  }
};
static_assert(ValidPeripheralConfiguration<SystickConfiguration>);

template <size_t INDEX>
struct I2cConfiguration;

template <>
struct I2cConfiguration<0> {
  using accessor = I2cAccessor<0>;

  static constexpr Facility facility() noexcept { return Facility::I2C; }
  static constexpr size_t index() noexcept { return 0; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table) noexcept {
    table.isrs[I2C0_EV_IRQn] = &i2c0_ev_isr;
    table.isrs[I2C0_ER_IRQn] = &i2c0_er_isr;
  }
};
static_assert(ValidPeripheralConfiguration<I2cConfiguration<0>>);

template <>
struct I2cConfiguration<1> {
  using accessor = I2cAccessor<1>;

  static constexpr Facility facility() noexcept { return Facility::I2C; }
  static constexpr size_t index() noexcept { return 1; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table) noexcept {
    table.isrs[I2C1_EV_IRQn] = &i2c1_ev_isr;
    table.isrs[I2C1_ER_IRQn] = &i2c1_er_isr;
  }
};
static_assert(ValidPeripheralConfiguration<I2cConfiguration<1>>);

TEST(SystemBuilderTest, CanConstructSystem) {
  auto system_builder{SystemBuilder<NUM_ISRS>{}
                          .add<RccConfiguration>()
                          .add<SystickConfiguration>()
                          .add<I2cConfiguration<0>>()
                          .add<I2cConfiguration<1>>()};

  auto sys{system_builder.system()};

  EXPECT_EQ(sys.rcc(), 100);
  EXPECT_EQ(sys.systick(), 200);
  EXPECT_EQ(sys.i2c0(), 300);
  EXPECT_EQ(sys.i2c1(), 400);
}

TEST(SystemBuilderTest, CanCreateSystemAtCompileTime) {
  constexpr auto system_builder{SystemBuilder<NUM_ISRS>{}
                                    .add<RccConfiguration>()
                                    .add<SystickConfiguration>()
                                    .add<I2cConfiguration<0>>()
                                    .add<I2cConfiguration<1>>()};

  // By using static constexpr here, we force the initialization of the system at compile-time.
  // Because the sys object is const, we can also verify compile-time initialization with
  // static_assert.
  static constexpr auto sys{system_builder.system()};
  static_assert(sys.initialized_at_compile_time);
  EXPECT_TRUE(sys.initialized_at_compile_time);
}

TEST(SystemBuilderTest, CanCreateModifiableSystemAtCompileTime) {
  constexpr auto system_builder{SystemBuilder<NUM_ISRS>{}
                                    .add<RccConfiguration>()
                                    .add<SystickConfiguration>()
                                    .add<I2cConfiguration<0>>()
                                    .add<I2cConfiguration<1>>()};

  // Note that we are using constinit to force initialization at compile-time. The resulting object
  // is not const, though.
  static constinit auto sys{system_builder.system()};
  EXPECT_TRUE(sys.initialized_at_compile_time);
  sys.modifiable_value = 3;
  EXPECT_EQ(sys.modifiable_value, 3);
}

TEST(SystemBuilderTest, CanCreateVectorTable) {
  auto system_builder{SystemBuilder<NUM_ISRS>{}
                          .add<RccConfiguration>()
                          .add<SystickConfiguration>()
                          .add<I2cConfiguration<0>>()
                          .add<I2cConfiguration<1>>()};

  auto vector_table{system_builder.create_vector_table()};

  EXPECT_EQ(vector_table.isrs[RCC_IRQn], &rcc_isr);
  EXPECT_EQ(vector_table.isrs[SYSTICK_IRQn], &systick_isr);
  EXPECT_EQ(vector_table.isrs[I2C0_EV_IRQn], &i2c0_ev_isr);
  EXPECT_EQ(vector_table.isrs[I2C0_ER_IRQn], &i2c0_er_isr);
  EXPECT_EQ(vector_table.isrs[I2C1_EV_IRQn], &i2c1_ev_isr);
  EXPECT_EQ(vector_table.isrs[I2C1_ER_IRQn], &i2c1_er_isr);
}

TEST(SystemBuilderTest, CanCreateVectorTableAtCompileTime) {
  static constexpr auto system_builder{SystemBuilder<NUM_ISRS>{}
                                           .add<RccConfiguration>()
                                           .add<SystickConfiguration>()
                                           .add<I2cConfiguration<0>>()
                                           .add<I2cConfiguration<1>>()};

  static constexpr auto vector_table{system_builder.create_vector_table()};
  static_assert(vector_table.isrs[RCC_IRQn] == &rcc_isr);
}

}  // namespace tvsc::hal::config
