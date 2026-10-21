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

template <typename System, typename PeripheralRegistry, size_t I2C_INDEX>
struct ImuAccessor {
  using I2cBus =  //
      typename PeripheralRegistry::template configuration<
          config::Facility::I2C_FACILITY, I2C_INDEX>::accessor<System, PeripheralRegistry>;

  int modifiable_value{42};
  bool initialized_at_compile_time{std::is_constant_evaluated()};

  // Production accessors would return an actual peripheral here, not an int.
  int imu0() {
    auto i2c_accessor{static_cast<I2cBus&>(static_cast<System&>(*this))};
    return i2c_accessor.get() + 100;
  }
  int get() { return imu0(); }
};

template <typename System>
struct SystickAccessor {
  int systick() { return 200; }
  int get() { return systick(); }
};

template <typename System, size_t INDEX>
struct I2cAccessor;

template <typename System>
struct I2cAccessor<System, 0> {
  constexpr int i2c0() const noexcept { return 300; }
  constexpr int get() const noexcept { return i2c0(); }
};

template <typename System>
struct I2cAccessor<System, 1> {
  constexpr int i2c1() const noexcept { return 400; }
  constexpr int get() const noexcept { return i2c1(); }
};

template <typename System>
struct RccAccessor {
  int rcc() { return 1000; }
  int get() { return rcc(); }
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

template <config::Facility FACILITY, size_t INDEX>
struct PeripheralConfiguration;

template <>
struct PeripheralConfiguration<Facility::IMU_FACILITY, 0> {
  static constexpr size_t I2C_INDEX{1};

  template <typename System, typename PeripheralRegistry>
  using accessor = ImuAccessor<System, PeripheralRegistry, I2C_INDEX>;

  static constexpr PeripheralId id() noexcept { return {Facility::IMU_FACILITY, 0}; }
  static constexpr auto dependencies() noexcept {
    return std::array{PeripheralId{Facility::I2C_FACILITY, I2C_INDEX}};
  }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table) noexcept {
    // No ISRs required.
  }
};

template <>
struct PeripheralConfiguration<Facility::SYSTICK_FACILITY, 0> {
  template <typename System, typename PeripheralRegistry>
  using accessor = SystickAccessor<System>;

  static constexpr PeripheralId id() noexcept { return {Facility::SYSTICK_FACILITY, 0}; }
  static constexpr auto dependencies() noexcept {
    return std::array{PeripheralId{Facility::RCC_FACILITY, 0}};
  }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table) noexcept {
    table.add_arm_isr(SYSTICK_IRQn, &systick_isr);
  }
};

template <size_t INDEX>
struct I2cConfiguration;

template <size_t INDEX>
struct PeripheralConfiguration<Facility::I2C_FACILITY, INDEX> {
  template <typename System, typename PeripheralRegistry>
  using accessor = I2cAccessor<System, INDEX>;

  static constexpr PeripheralId id() noexcept { return {Facility::I2C_FACILITY, INDEX}; }
  static constexpr auto dependencies() noexcept {
    return std::array{PeripheralId{Facility::SYSTICK_FACILITY, 0},
                      PeripheralId{Facility::RCC_FACILITY, 0}};
  }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table) noexcept {
    if constexpr (INDEX == 0) {
      table.add_arm_isr(I2C0_EV_IRQn, &i2c0_ev_isr);
      table.add_arm_isr(I2C0_ER_IRQn, &i2c0_er_isr);
    } else if constexpr (INDEX == 1) {
      table.add_arm_isr(I2C1_EV_IRQn, &i2c1_ev_isr);
      table.add_arm_isr(I2C1_ER_IRQn, &i2c1_er_isr);
    }
  }
};

template <>
struct PeripheralConfiguration<Facility::RCC_FACILITY, 0> {
  template <typename System, typename PeripheralRegistry>
  using accessor = RccAccessor<System>;

  static constexpr PeripheralId id() noexcept { return {Facility::RCC_FACILITY, 0}; }
  static constexpr auto dependencies() noexcept { return std::array<PeripheralId, 0>{}; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table) noexcept {
    // No ISRs.
  }
};

struct Board final {
  template <config::Facility FACILITY, size_t INDEX>
  using configuration = PeripheralConfiguration<FACILITY, INDEX>;
};

static_assert(
    ValidPeripheralConfiguration<PeripheralConfiguration<Facility::IMU_FACILITY, 0>, Board>);
static_assert(
    ValidPeripheralConfiguration<PeripheralConfiguration<Facility::SYSTICK_FACILITY, 0>, Board>);
static_assert(
    ValidPeripheralConfiguration<PeripheralConfiguration<Facility::I2C_FACILITY, 0>, Board>);
static_assert(
    ValidPeripheralConfiguration<PeripheralConfiguration<Facility::I2C_FACILITY, 1>, Board>);
static_assert(
    ValidPeripheralConfiguration<PeripheralConfiguration<Facility::RCC_FACILITY, 0>, Board>);

TEST(SystemBuilderTest, CanConstructSystem) {
  auto system_builder{SystemBuilder<Board>{}
                          .add<Facility::RCC_FACILITY, 0>()
                          .add<Facility::SYSTICK_FACILITY, 0>()
                          .add<Facility::I2C_FACILITY, 0>()
                          .add<Facility::I2C_FACILITY, 1>()
                          .add<Facility::IMU_FACILITY, 0>()};

  auto sys{system_builder.system()};

  EXPECT_EQ(sys.rcc(), 1000);
  EXPECT_EQ(sys.systick(), 200);
  EXPECT_EQ(sys.i2c0(), 300);
  EXPECT_EQ(sys.i2c1(), 400);
  EXPECT_EQ(sys.imu0(), sys.i2c1() + 100);
}

TEST(SystemBuilderTest, IgnoresDuplicates) {
  auto system_builder{SystemBuilder<Board>{}
                          .add<Facility::I2C_FACILITY, 0>()
                          .add<Facility::I2C_FACILITY, 1>()
                          .add<Facility::I2C_FACILITY, 0>()
                          .add<Facility::IMU_FACILITY, 0>()};

  auto sys{system_builder.system()};

  EXPECT_EQ(sys.i2c0(), 300);
  EXPECT_EQ(sys.i2c1(), 400);
  EXPECT_EQ(sys.imu0(), sys.i2c1() + 100);
}

TEST(SystemBuilderTest, AddsDependencies) {
  auto system_builder{
      SystemBuilder<Board>{}                     //
          .add<Facility::SYSTICK_FACILITY, 0>()  //
          .add<Facility::IMU_FACILITY, 0>()      //
  };

  auto sys{system_builder.system()};

  EXPECT_EQ(sys.systick(), 200);
  EXPECT_EQ(sys.i2c1(), 400);
  EXPECT_EQ(sys.imu0(), sys.i2c1() + 100);
}

TEST(SystemBuilderTest, AddsDependenciesTransitively) {
  auto system_builder{
      SystemBuilder<Board>{}                     //
          .add<Facility::SYSTICK_FACILITY, 0>()  //
          .add<Facility::IMU_FACILITY, 0>()      //
  };

  auto sys{system_builder.system()};

  EXPECT_EQ(sys.systick(), 200);
  EXPECT_EQ(sys.i2c1(), 400);
  EXPECT_EQ(sys.imu0(), sys.i2c1() + 100);
  EXPECT_EQ(sys.rcc(), 1000);
}

TEST(SystemBuilderTest, CanCreateSystemAtCompileTime) {
  constexpr auto system_builder{SystemBuilder<Board>{}
                                    .add<Facility::IMU_FACILITY, 0>()
                                    .add<Facility::SYSTICK_FACILITY, 0>()
                                    .add<Facility::I2C_FACILITY, 0>()
                                    .add<Facility::I2C_FACILITY, 1>()};

  // By using static constexpr here, we force the initialization of the system at compile-time.
  // Because the sys object is const, we can also verify compile-time initialization with
  // static_assert.
  static constexpr auto sys{system_builder.system()};
  static_assert(sys.initialized_at_compile_time);
  EXPECT_TRUE(sys.initialized_at_compile_time);
}

TEST(SystemBuilderTest, CanAddDependenciesAtCompileTime) {
  constexpr auto system_builder{
      SystemBuilder<Board>{}                     //
          .add<Facility::IMU_FACILITY, 0>()      //
          .add<Facility::SYSTICK_FACILITY, 0>()  //
  };

  // By using static constexpr here, we force the initialization of the system at compile-time.
  // Because the sys object is const, we can also verify compile-time initialization with
  // static_assert.
  static constexpr auto sys{system_builder.system()};
  static_assert(sys.initialized_at_compile_time);
  EXPECT_TRUE(sys.initialized_at_compile_time);
}

TEST(SystemBuilderTest, CanCreateModifiableSystemAtCompileTime) {
  constexpr auto system_builder{SystemBuilder<Board>{}
                                    .add<Facility::IMU_FACILITY, 0>()
                                    .add<Facility::SYSTICK_FACILITY, 0>()
                                    .add<Facility::I2C_FACILITY, 0>()
                                    .add<Facility::I2C_FACILITY, 1>()};

  // Note that we are using constinit to force initialization at compile-time. The resulting object
  // is not const, though.
  static constinit auto sys{system_builder.system()};
  EXPECT_TRUE(sys.initialized_at_compile_time);
  sys.modifiable_value = 3;
  EXPECT_EQ(sys.modifiable_value, 3);
}

TEST(SystemBuilderTest, CanCreateModifiableSystemWhileAddingDependenciesTransitivelyAtCompileTime) {
  constexpr auto system_builder{
      SystemBuilder<Board>{}                     //
          .add<Facility::IMU_FACILITY, 0>()      //
          .add<Facility::SYSTICK_FACILITY, 0>()  //
  };

  // Note that we are using constinit to force initialization at compile-time. The resulting object
  // is not const, though.
  static constinit auto sys{system_builder.system()};
  EXPECT_TRUE(sys.initialized_at_compile_time);
  sys.modifiable_value = 3;
  EXPECT_EQ(sys.modifiable_value, 3);
  EXPECT_EQ(sys.imu0(), sys.i2c1() + 100);
  EXPECT_EQ(sys.rcc(), 1000);
}

TEST(SystemBuilderTest, CanCreateVectorTable) {
  auto system_builder{SystemBuilder<Board>{}
                          .add<Facility::IMU_FACILITY, 0>()
                          .add<Facility::SYSTICK_FACILITY, 0>()
                          .add<Facility::I2C_FACILITY, 0>()
                          .add<Facility::I2C_FACILITY, 1>()};

  auto vector_table{system_builder.create_vector_table<NUM_ISRS>()};

  EXPECT_EQ(vector_table.at_arm_isr(SYSTICK_IRQn), &systick_isr);
  EXPECT_EQ(vector_table.at_arm_isr(I2C0_EV_IRQn), &i2c0_ev_isr);
  EXPECT_EQ(vector_table.at_arm_isr(I2C0_ER_IRQn), &i2c0_er_isr);
  EXPECT_EQ(vector_table.at_arm_isr(I2C1_EV_IRQn), &i2c1_ev_isr);
  EXPECT_EQ(vector_table.at_arm_isr(I2C1_ER_IRQn), &i2c1_er_isr);
}

TEST(SystemBuilderTest, CanCreateVectorTableWhileAddingDependencies) {
  auto system_builder{
      SystemBuilder<Board>{}                 //
          .add<Facility::IMU_FACILITY, 0>()  //
  };

  auto vector_table{system_builder.create_vector_table<NUM_ISRS>()};

  EXPECT_EQ(vector_table.at_arm_isr(SYSTICK_IRQn), &systick_isr);
  EXPECT_EQ(vector_table.at_arm_isr(I2C1_EV_IRQn), &i2c1_ev_isr);
  EXPECT_EQ(vector_table.at_arm_isr(I2C1_ER_IRQn), &i2c1_er_isr);
}

TEST(SystemBuilderTest, CanCreateVectorTableAtCompileTime) {
  static constexpr auto system_builder{SystemBuilder<Board>{}
                                           .add<Facility::SYSTICK_FACILITY, 0>()
                                           .add<Facility::I2C_FACILITY, 0>()
                                           .add<Facility::I2C_FACILITY, 1>()
                                           .add<Facility::IMU_FACILITY, 0>()};

  static constexpr auto vector_table{system_builder.create_vector_table<NUM_ISRS>()};
  static_assert(vector_table.at_arm_isr(SYSTICK_IRQn) == &systick_isr);
}

TEST(SystemBuilderTest, CanCreateVectorTableWhileAddingDependenciesAtCompileTime) {
  static constexpr auto system_builder{
      SystemBuilder<Board>{}                 //
          .add<Facility::IMU_FACILITY, 0>()  //
  };

  static constexpr auto vector_table{system_builder.create_vector_table<NUM_ISRS>()};
  static_assert(vector_table.at_arm_isr(SYSTICK_IRQn) == &systick_isr);
  static_assert(vector_table.at_arm_isr(I2C1_EV_IRQn) == &i2c1_ev_isr);
  static_assert(vector_table.at_arm_isr(I2C1_ER_IRQn) == &i2c1_er_isr);
}

}  // namespace tvsc::hal2::config
