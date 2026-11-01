#include "hal2/config/system_builder.h"

#include <cstddef>

#include "gtest/gtest.h"
#include "hal2/config/device_definitions.h"
#include "hal2/config/vector_table.h"
#include "hal2/device.h"

// Symbols provided by startup assembly and the linker. These empty versions are provided here to
// make the tests work.
extern "C" {
char _estack[0];
void Reset_Handler(void) {}
}

namespace tvsc::hal2::config {

template <typename System, typename DeviceRegistry, size_t I2C_INDEX>
struct ImuAccessor {
  using I2cBus =
      config::accessor_type<System, DeviceRegistry, DeviceId{Facility::I2C_FACILITY, I2C_INDEX}>;

  int modifiable_value{42};
  bool initialized_at_compile_time{std::is_constant_evaluated()};

  // Production accessors would return an actual device here, not an int.
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

template <Facility FACILITY, size_t INDEX>
struct DeviceDefinition;

template <>
struct DeviceDefinition<Facility::IMU_FACILITY, 0> : public UnconfigurableDevice {
  static constexpr size_t I2C_INDEX{1};

  template <typename System, typename DeviceRegistry, configuration CONFIG>
  using accessor = ImuAccessor<System, DeviceRegistry, I2C_INDEX>;

  [[nodiscard]] static constexpr DeviceId id() noexcept { return {Facility::IMU_FACILITY, 0}; }
  [[nodiscard]] static constexpr const char* name() noexcept { return "imu"; }
  [[nodiscard]] static constexpr auto dependencies(const configuration&) noexcept {
    return std::array{DeviceId{Facility::I2C_FACILITY, I2C_INDEX}};
  }
};

template <>
struct DeviceDefinition<Facility::SYSTICK_FACILITY, 0> : public UnconfigurableDevice {
  template <typename System, typename DeviceRegistry, configuration CONFIG>
  using accessor = SystickAccessor<System>;

  static constexpr DeviceId id() noexcept { return {Facility::SYSTICK_FACILITY, 0}; }
  [[nodiscard]] static constexpr const char* name() noexcept { return "systick"; }
  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table,
                                            const configuration&) noexcept {
    table.add_arm_isr(SYSTICK_IRQn, &systick_isr);
  }
};

template <size_t INDEX>
struct I2cConfiguration;

template <size_t INDEX>
struct DeviceDefinition<Facility::I2C_FACILITY, INDEX> {
  static constexpr DeviceId id() noexcept { return {Facility::I2C_FACILITY, INDEX}; }
  [[nodiscard]] static constexpr const char* name() noexcept {
    if constexpr (INDEX == 0) {
      return "i2c0";
    } else if constexpr (INDEX == 1) {
      return "i2c1";
    }
  }

  struct Configuration final {
    int sda_pin{-1};
    int sck_pin{-1};
  };
  using configuration = Configuration;

  template <typename System, typename DeviceRegistry, configuration CONFIG>
  using accessor = I2cAccessor<System, INDEX>;

  static constexpr bool is_valid(const configuration&) noexcept { return true; }
  static constexpr auto dependencies(const configuration&) noexcept {
    return std::array{DeviceId{Facility::SYSTICK_FACILITY, 0}, DeviceId{Facility::RCC_FACILITY, 0}};
  }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(VectorTable<NUM_ISRS>& table,
                                            const configuration&) noexcept {
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
struct DeviceDefinition<Facility::RCC_FACILITY, 0> : public UnconfigurableDevice {
  template <typename System, typename DeviceRegistry, configuration CONFIG>
  using accessor = RccAccessor<System>;

  static constexpr DeviceId id() noexcept { return {Facility::RCC_FACILITY, 0}; }
  static constexpr const char* name() noexcept { return "rcc"; }
  static constexpr auto dependencies(const configuration&) noexcept {
    return std::array<DeviceId, 0>{};
  }
};

struct Board final {
  template <Facility FACILITY, size_t INDEX>
  using definition = DeviceDefinition<FACILITY, INDEX>;
};

static_assert(ValidDeviceDefinition<DeviceDefinition<Facility::IMU_FACILITY, 0>, Board>);
static_assert(ValidDeviceDefinition<DeviceDefinition<Facility::SYSTICK_FACILITY, 0>, Board>);
static_assert(ValidDeviceDefinition<DeviceDefinition<Facility::I2C_FACILITY, 0>, Board>);
static_assert(ValidDeviceDefinition<DeviceDefinition<Facility::I2C_FACILITY, 1>, Board>);
static_assert(ValidDeviceDefinition<DeviceDefinition<Facility::RCC_FACILITY, 0>, Board>);

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

  // Note that we are using constinit to force initialization at compile-time. The resulting
  // object is not const, though.
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

  // Note that we are using constinit to force initialization at compile-time. The resulting
  // object is not const, though.
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
