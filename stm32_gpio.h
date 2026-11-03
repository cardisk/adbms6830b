/***************************************************************************//**
 *   @file   stm32_gpio.h
 *   @brief  Header file for STM32 GPIO platform driver.
 *   @author Darius Berghe (darius.berghe@analog.com)
********************************************************************************
 * Copyright 2020(c) Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#ifndef _STM32_GPIO_H_
#define _STM32_GPIO_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32_hal.h"
#include "no_os_gpio.h"

struct stm32_gpio_init_param {
	uint32_t mode;
	uint32_t speed;
	uint32_t alternate;
};

struct stm32_gpio_desc {
	GPIO_TypeDef *port;
	GPIO_InitTypeDef gpio_config;
};

extern const struct no_os_gpio_platform_ops stm32_gpio_ops;

#endif /* _STM32_GPIO_H_ */
