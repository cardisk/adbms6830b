/***************************************************************************//**
 *   @file   no_os_gpio.c
 *   @brief  Implementation of the GPIO Interface dispatcher.
 *   @author Antoniu Miclaus (antoniu.miclaus@analog.com)
********************************************************************************
 * Copyright 2020(c) Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#include "no_os_gpio.h"
#include "no_os_error.h"

int32_t no_os_gpio_get(struct no_os_gpio_desc **desc,
		       const struct no_os_gpio_init_param *param)
{
	if (!param || !param->platform_ops || !param->platform_ops->gpio_ops_get)
		return -EINVAL;

	return param->platform_ops->gpio_ops_get(desc, param);
}

int32_t no_os_gpio_remove(struct no_os_gpio_desc *desc)
{
	if (!desc || !desc->platform_ops || !desc->platform_ops->gpio_ops_remove)
		return -EINVAL;

	return desc->platform_ops->gpio_ops_remove(desc);
}

int32_t no_os_gpio_direction_input(struct no_os_gpio_desc *desc)
{
	if (!desc || !desc->platform_ops || !desc->platform_ops->gpio_ops_direction_input)
		return -EINVAL;

	return desc->platform_ops->gpio_ops_direction_input(desc);
}

int32_t no_os_gpio_direction_output(struct no_os_gpio_desc *desc, uint8_t value)
{
	if (!desc || !desc->platform_ops || !desc->platform_ops->gpio_ops_direction_output)
		return -EINVAL;

	return desc->platform_ops->gpio_ops_direction_output(desc, value);
}

int32_t no_os_gpio_get_direction(struct no_os_gpio_desc *desc, uint8_t *direction)
{
	if (!desc || !desc->platform_ops || !desc->platform_ops->gpio_ops_get_direction)
		return -EINVAL;

	return desc->platform_ops->gpio_ops_get_direction(desc, direction);
}

int32_t no_os_gpio_set_value(struct no_os_gpio_desc *desc, uint8_t value)
{
	if (!desc || !desc->platform_ops || !desc->platform_ops->gpio_ops_set_value)
		return -EINVAL;

	return desc->platform_ops->gpio_ops_set_value(desc, value);
}

int32_t no_os_gpio_get_value(struct no_os_gpio_desc *desc, uint8_t *value)
{
	if (!desc || !desc->platform_ops || !desc->platform_ops->gpio_ops_get_value)
		return -EINVAL;

	return desc->platform_ops->gpio_ops_get_value(desc, value);
}
