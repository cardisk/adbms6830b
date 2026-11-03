/***************************************************************************//**
 *   @file   stm32_gpio.c
 *   @brief  Implementation of STM32 GPIO driver.
 *   @author Darius Berghe (darius.berghe@analog.com)
********************************************************************************
 * Copyright 2020(c) Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#include <stdlib.h>
#include "no_os_util.h"
#include "no_os_gpio.h"
#include "no_os_alloc.h"
#include "no_os_error.h"
#include "stm32_gpio.h"

static GPIO_TypeDef *const stm32_gpio_ports[] = {
#ifdef GPIOA
	GPIOA,
#else
	NULL,
#endif
#ifdef GPIOB
	GPIOB,
#else
	NULL,
#endif
#ifdef GPIOC
	GPIOC,
#else
	NULL,
#endif
#ifdef GPIOD
	GPIOD,
#else
	NULL,
#endif
#ifdef GPIOE
	GPIOE,
#else
	NULL,
#endif
};

static int32_t stm32_gpio_get_fn(struct no_os_gpio_desc **desc,
				 const struct no_os_gpio_init_param *param)
{
	struct no_os_gpio_desc *descriptor;
	struct stm32_gpio_desc *extra;

	if (!desc || !param)
		return -EINVAL;

	if (param->port >= (int32_t)NO_OS_ARRAY_SIZE(stm32_gpio_ports) ||
	    !stm32_gpio_ports[param->port])
		return -EINVAL;

	descriptor = (struct no_os_gpio_desc *)no_os_calloc(1, sizeof(*descriptor));
	if (!descriptor)
		return -ENOMEM;

	extra = (struct stm32_gpio_desc *)no_os_calloc(1, sizeof(*extra));
	if (!extra) {
		no_os_free(descriptor);
		return -ENOMEM;
	}

	descriptor->port = param->port;
	descriptor->number = param->number;
	descriptor->pull = param->pull;
	descriptor->platform_ops = param->platform_ops;
	descriptor->extra = extra;

	extra->port = stm32_gpio_ports[param->port];
	*desc = descriptor;

	return 0;
}

static int32_t stm32_gpio_remove_fn(struct no_os_gpio_desc *desc)
{
	if (!desc)
		return -EINVAL;

	if (desc->extra)
		no_os_free(desc->extra);
	no_os_free(desc);

	return 0;
}

static int32_t stm32_gpio_set_value_fn(struct no_os_gpio_desc *desc, uint8_t value)
{
	struct stm32_gpio_desc *extra;

	if (!desc || !desc->extra)
		return -EINVAL;

	extra = desc->extra;
	extra->port->BSRR = (uint32_t)NO_OS_BIT(desc->number) << (value ? 0 : 16);

	return 0;
}

static int32_t stm32_gpio_get_value_fn(struct no_os_gpio_desc *desc, uint8_t *value)
{
	struct stm32_gpio_desc *extra;

	if (!desc || !desc->extra || !value)
		return -EINVAL;

	extra = desc->extra;
	*value = (extra->port->IDR & NO_OS_BIT(desc->number)) ? NO_OS_GPIO_HIGH : NO_OS_GPIO_LOW;

	return 0;
}

static int32_t stm32_gpio_direction_input_fn(struct no_os_gpio_desc *desc)
{
	struct stm32_gpio_desc *extra;
	GPIO_InitTypeDef init = {0};

	if (!desc || !desc->extra)
		return -EINVAL;

	extra = desc->extra;
	init.Pin = NO_OS_BIT(desc->number);
	init.Mode = GPIO_MODE_INPUT;
	init.Pull = (desc->pull == NO_OS_PULL_UP) ? GPIO_PULLUP :
		    (desc->pull == NO_OS_PULL_DOWN) ? GPIO_PULLDOWN : GPIO_NOPULL;
	HAL_GPIO_Init(extra->port, &init);

	return 0;
}

static int32_t stm32_gpio_direction_output_fn(struct no_os_gpio_desc *desc, uint8_t value)
{
	struct stm32_gpio_desc *extra;
	GPIO_InitTypeDef init = {0};

	if (!desc || !desc->extra)
		return -EINVAL;

	extra = desc->extra;
	stm32_gpio_set_value_fn(desc, value);

	init.Pin = NO_OS_BIT(desc->number);
	init.Mode = GPIO_MODE_OUTPUT_PP;
	init.Pull = GPIO_NOPULL;
	init.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(extra->port, &init);

	return 0;
}

static int32_t stm32_gpio_get_direction_fn(struct no_os_gpio_desc *desc, uint8_t *direction)
{
	(void)desc;
	(void)direction;
	return -ENOSYS;
}

const struct no_os_gpio_platform_ops stm32_gpio_ops = {
	.gpio_ops_get = stm32_gpio_get_fn,
	.gpio_ops_remove = stm32_gpio_remove_fn,
	.gpio_ops_direction_input = stm32_gpio_direction_input_fn,
	.gpio_ops_direction_output = stm32_gpio_direction_output_fn,
	.gpio_ops_get_direction = stm32_gpio_get_direction_fn,
	.gpio_ops_set_value = stm32_gpio_set_value_fn,
	.gpio_ops_get_value = stm32_gpio_get_value_fn,
};
