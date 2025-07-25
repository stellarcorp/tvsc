#include "hal/rcc/stm32l4xx_rcc.h"

#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::rcc {

void RccStm32L4xx::set_clock_to_max_speed() {
  // The current implementation puts SYSCLK signal as well as peripheral buses at 80 MHz. This speed
  // requires four flash wait states per operation.

  __HAL_RCC_HSI_ENABLE();
  while (__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == 0) {
    // Busy loop until the oscillator is available.
  }

  // Bring up the internal regulator voltage to its normal level. The system will be very unstable
  // if we are undervolting while trying to run at a higher clock speed.
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  // The HSI oscillator is a non-configurable 16 MHz, which is the speed we want for efficiency.
  RCC_OscInitTypeDef RCC_OscInitStruct{};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;

  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  // Configure the flash latency as well as initialize the CPU, AHB and APB bus clocks to use the
  // SYSCLK and HCLK without any dividers.
  RCC_ClkInitTypeDef RCC_ClkInitStruct{};
  RCC_ClkInitStruct.ClockType =
      RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  // Note that we also designate the flash latency here as having a one cycle wait state.
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);

  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);

  __HAL_RCC_MSI_DISABLE();

  clock_configuration_ = ClockConfiguration::MAX_SPEED;
}

void RccStm32L4xx::set_clock_to_energy_efficient_speed() {
  // The current implementation puts SYSCLK signal as well as peripheral buses at 16 MHz. This speed
  // is the maximum speed allowed when undervolting the system. This speed requires zero flash wait
  // state per operation.

  __HAL_RCC_HSI_ENABLE();
  while (__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == 0) {
    // Busy loop until the oscillator is available.
  }

  // The HSI oscillator is a non-configurable 16 MHz, which is the speed we want for efficiency.
  RCC_OscInitTypeDef RCC_OscInitStruct{};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  // Configure the flash latency as well as initialize the CPU, AHB and APB bus clocks to use the
  // MSI without any divider.
  RCC_ClkInitTypeDef RCC_ClkInitStruct{};
  RCC_ClkInitStruct.ClockType =
      RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  // Note that we also designate the flash latency here as having a one cycle wait state.
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);

  // Configure the main internal regulator output voltage to undervolt the CPU to save power.
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);

  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);

  __HAL_RCC_MSI_DISABLE();

  clock_configuration_ = ClockConfiguration::ENERGY_EFFICIENT_SPEED;
}

void RccStm32L4xx::set_clock_to_min_speed() {
  __HAL_RCC_MSI_ENABLE();
  while (__HAL_RCC_GET_FLAG(RCC_FLAG_MSIRDY) == 0) {
    // Busy loop until the oscillator is available.
  }

  // Set the MSI oscillator to 100 kHz. This is its minimum speed.
  // Bypass the PLL.
  RCC_OscInitTypeDef RCC_OscInitStruct{};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_0;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  // Configure the flash latency as well as initialize the CPU, AHB and APB buses clocks to use the
  // MSI without any divider.
  RCC_ClkInitTypeDef RCC_ClkInitStruct{};
  RCC_ClkInitStruct.ClockType =
      RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  // Note that we also designate the flash latency here as having zero wait states.
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);

  // Configure the main internal regulator output voltage to undervolt the CPU to save power.
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);

  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);

  __HAL_RCC_HSI_DISABLE();

  clock_configuration_ = ClockConfiguration::MIN_SPEED;
}

void RccStm32L4xx::restore_clock_speed() {
  if (clock_configuration_ == ClockConfiguration::MIN_SPEED) {
    set_clock_to_min_speed();
  } else if (clock_configuration_ == ClockConfiguration::ENERGY_EFFICIENT_SPEED) {
    set_clock_to_energy_efficient_speed();
  } else if (clock_configuration_ == ClockConfiguration::MAX_SPEED) {
    set_clock_to_max_speed();
  }
}

void Hsi48OscillatorStm32L4xx::enable() {
  /**
   * Turn up the main internal regulator output voltage to 1.2V. This allows clock speeds up to
   * 80MHz, but it uses more power.
   */
  // TODO(james): Determine if this is necessary. The HSI 48 oscillator may run independent of this
  // voltage regulator.
  // HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /**
   * Turn on the HSI 48 oscillator.
   */
  RCC_OscInitTypeDef RCC_OscInitStruct{};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  // Note that this function call can block for up to 2 ms.
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
}

void Hsi48OscillatorStm32L4xx::disable() {
  /**
   * Turn off the HSI 48 oscillator.
   */
  RCC_OscInitTypeDef RCC_OscInitStruct{};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_OFF;
  // Note that this function call can block for up to 2 ms.
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /**
   * Turn down the main internal regulator output voltage to 1.0V. This restricts clock speeds to
   * under 26MHz and conserves power.
   */
  // TODO(james): Determine if this is necessary. The HSI 48 oscillator may run independent of this
  // voltage regulator.
  // HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);
}

void LsiOscillatorStm32L4xx::enable() {
  // Note that these might be needed, both in enable() and disable(). Not sure why, or if they
  // belong here, or in the power module.
  // __HAL_RCC_PWR_CLK_ENABLE();
  // HAL_PWR_EnableBkUpAccess();

  /**
   * Turn on the LSI oscillator.
   */
  RCC_OscInitTypeDef RCC_OscInitStruct{};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  // Note that this function call can block for up to 2 ms.
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
}

void LsiOscillatorStm32L4xx::disable() {
  /**
   * Turn off the LSI oscillator.
   */
  RCC_OscInitTypeDef RCC_OscInitStruct{};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
  // Note that this function call can block for up to 2 ms.
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  // Note that these might be needed, both in enable() and disable(). Not sure why, or if they
  // belong here, or in the power module.
  //  HAL_PWR_DisableBkUpAccess();
  // __HAL_RCC_PWR_CLK_DISABLE();
}

}  // namespace tvsc::hal::rcc
