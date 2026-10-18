#include "hal2/config/system_builder.h"

#include <cstddef>

#include "gtest/gtest.h"
#include "hal2/config/facility.h"
#include "hal2/config/peripheral_accessors.h"
#include "hal2/config/vector_table.h"

// Symbols provided by startup assembly and the linker. These empty versions are provided here to
// make the tests work.
extern "C" {
char _estack[0];
void Reset_Handler(void) {}
}

namespace tvsc::hal2::config {

template <typename System, typename I2cConfiguration>
struct ImuAccessor {
  int modifiable_value{42};
  bool initialized_at_compile_time{std::is_constant_evaluated()};

  // Production accessors would return an actual peripheral here, not an int.
  int imu0() {
    auto i2c_accessor{
        static_cast<I2cConfiguration::template accessor<System>&>(static_cast<System&>(*this))};
    return i2c_accessor.get() + 100;
  }
  int get() { return imu0(); }
};

template <typename System>
struct SystickAccessor {
  int systick() { return 200; }
  int get() { return systick(); }
};

template <size_t INDEX, typename System>
struct I2cAccessor;

template <typename System>
struct I2cAccessor<0, System> {
  constexpr int i2c0() const noexcept { return 300; }
  constexpr int get() const noexcept { return i2c0(); }
};

template <typename System>
struct I2cAccessor<1, System> {
  static constexpr Facility facility() noexcept { return Facility::I2C; }
  static constexpr size_t index() noexcept { return 1; }

  constexpr int i2c1() const noexcept { return 400; }
  constexpr int get() const noexcept { return i2c1(); }
};

inline void systick_isr() noexcept {}
inline void i2c0_ev_isr() noexcept {}
inline void i2c0_er_isr() noexcept {}
inline void i2c1_ev_isr() noexcept {}
inline void i2c1_er_isr() noexcept {}

static constexpr size_t NUM_ISRS{23};
// Note that these are test values, not the real interrupt numbers, but yes, the real values (on ARM
// at least) can be negative.
static constexpr int SYSTICK_IRQn{-3};
static constexpr int I2C0_EV_IRQn{4};
static constexpr int I2C0_ER_IRQn{6};
static constexpr int I2C1_EV_IRQn{5};
static constexpr int I2C1_ER_IRQn{7};

template <typename I2cConfiguration>
struct ImuConfiguration {
  template <typename System>
  using accessor = ImuAccessor<System, I2cConfiguration>;

  static constexpr Facility facility() noexcept { return Facility::IMU; }
  static constexpr size_t index() noexcept { return 0; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table) noexcept {
    // No ISRs required.
  }
};
static_assert(ValidPeripheralConfiguration<ImuConfiguration<int>>);

struct SystickConfiguration {
  template <typename System>
  using accessor = SystickAccessor<System>;

  static constexpr Facility facility() noexcept { return Facility::SYSTICK; }
  static constexpr size_t index() noexcept { return 0; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table) noexcept {
    table.add_arm_isr(SYSTICK_IRQn, &systick_isr);
  }
};
static_assert(ValidPeripheralConfiguration<SystickConfiguration>);

template <size_t INDEX>
struct I2cConfiguration;

template <>
struct I2cConfiguration<0> {
  template <typename System>
  using accessor = I2cAccessor<0, System>;

  static constexpr Facility facility() noexcept { return Facility::I2C; }
  static constexpr size_t index() noexcept { return 0; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table) noexcept {
    table.add_arm_isr(I2C0_EV_IRQn, &i2c0_ev_isr);
    table.add_arm_isr(I2C0_ER_IRQn, &i2c0_er_isr);
  }
};
static_assert(ValidPeripheralConfiguration<I2cConfiguration<0>>);

template <>
struct I2cConfiguration<1> {
  template <typename System>
  using accessor = I2cAccessor<1, System>;

  static constexpr Facility facility() noexcept { return Facility::I2C; }
  static constexpr size_t index() noexcept { return 1; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table) noexcept {
    table.add_arm_isr(I2C1_EV_IRQn, &i2c1_ev_isr);
    table.add_arm_isr(I2C1_ER_IRQn, &i2c1_er_isr);
  }
};
static_assert(ValidPeripheralConfiguration<I2cConfiguration<1>>);

TEST(SystemBuilderTest, CanConstructSystem) {
  auto system_builder{SystemBuilder{}
                          .add<SystickConfiguration>()
                          .add<I2cConfiguration<0>>()
                          .add<I2cConfiguration<1>>()
                          .add<ImuConfiguration<I2cConfiguration<1>>>()};

  auto sys{system_builder.system()};

  EXPECT_EQ(sys.systick(), 200);
  EXPECT_EQ(sys.i2c0(), 300);
  EXPECT_EQ(sys.i2c1(), 400);
  EXPECT_EQ(sys.imu0(), sys.i2c1() + 100);
}

TEST(SystemBuilderTest, CanCreateSystemAtCompileTime) {
  constexpr auto system_builder{SystemBuilder{}
                                    .add<ImuConfiguration<I2cConfiguration<1>>>()
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
  constexpr auto system_builder{SystemBuilder{}
                                    .add<ImuConfiguration<I2cConfiguration<1>>>()
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
  auto system_builder{SystemBuilder{}
                          .add<ImuConfiguration<I2cConfiguration<1>>>()
                          .add<SystickConfiguration>()
                          .add<I2cConfiguration<0>>()
                          .add<I2cConfiguration<1>>()};

  auto vector_table{system_builder.create_vector_table<NUM_ISRS>()};

  EXPECT_EQ(vector_table.at_arm_isr(SYSTICK_IRQn), &systick_isr);
  EXPECT_EQ(vector_table.at_arm_isr(I2C0_EV_IRQn), &i2c0_ev_isr);
  EXPECT_EQ(vector_table.at_arm_isr(I2C0_ER_IRQn), &i2c0_er_isr);
  EXPECT_EQ(vector_table.at_arm_isr(I2C1_EV_IRQn), &i2c1_ev_isr);
  EXPECT_EQ(vector_table.at_arm_isr(I2C1_ER_IRQn), &i2c1_er_isr);
}

TEST(SystemBuilderTest, CanCreateVectorTableAtCompileTime) {
  static constexpr auto system_builder{SystemBuilder{}
                                           .add<SystickConfiguration>()
                                           .add<I2cConfiguration<0>>()
                                           .add<I2cConfiguration<1>>()
                                           .add<ImuConfiguration<I2cConfiguration<1>>>()};

  static constexpr auto vector_table{system_builder.create_vector_table<NUM_ISRS>()};
  static_assert(vector_table.at_arm_isr(SYSTICK_IRQn) == &systick_isr);
}

}  // namespace tvsc::hal2::config
