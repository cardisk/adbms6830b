/***************************************************************************//**
 *   @file   stm32_delay.c
 *   @brief  Implementation of stm32 delay functions.
 *   @author Darius Berghe (darius.berghe@analog.com)
********************************************************************************
 * Copyright 2020(c) Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#include <stdbool.h>
#include "stm32_hal.h"
#include "no_os_delay.h"

/**
 * @brief Generate microseconds delay using Cortex-M DWT cycle counter.
 * @param usecs - Delay in microseconds.
 */
#if defined(DWT)
void no_os_udelay(uint32_t usecs)
{
	static bool firstrun = true;
	volatile uint32_t cycles = (SystemCoreClock / 1000000L) * usecs;

	if (firstrun) {
		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
#if defined(DWT_LAR_KEY) || defined(STM32F7) || defined(STM32H7)
		DWT->LAR = 0xC5ACCE55;
#endif
		DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
		firstrun = false;
	}

	volatile uint32_t start = DWT->CYCCNT;
	while ((DWT->CYCCNT - start) < cycles);
}
#else
void no_os_udelay(uint32_t usecs)
{
	/* Fallback: 1ms resolution */
	HAL_Delay((usecs + 999) / 1000);
}
#endif

/**
 * @brief Generate milliseconds delay using STM32 HAL tick.
 * @param msecs - Delay in milliseconds.
 */
void no_os_mdelay(uint32_t msecs)
{
	HAL_Delay(msecs);
}

/**
 * @brief Get current time structure from system start.
 * @return Current time structure (seconds, microseconds).
 */
struct no_os_time no_os_get_time(void)
{
	uint32_t ticks = HAL_GetTick();
	struct no_os_time t;

	t.s = ticks / 1000;
	t.us = (ticks % 1000) * 1000;

	return t;
}
