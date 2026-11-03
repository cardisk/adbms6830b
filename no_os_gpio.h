/***************************************************************************//**
 *   @file   no_os_gpio.h
 *   @brief  Header file of GPIO Interface
 *   @author DBogdan (dragos.bogdan@analog.com)
********************************************************************************
 * Copyright 2019(c) Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#ifndef _NO_OS_GPIO_H_
#define _NO_OS_GPIO_H_

#include <stdint.h>

#define NO_OS_GPIO_OUT	0x01
#define NO_OS_GPIO_IN	0x00

struct no_os_gpio_platform_ops;

enum no_os_gpio_pull_up {
	NO_OS_PULL_NONE,
	NO_OS_PULL_UP,
	NO_OS_PULL_DOWN,
	NO_OS_PULL_UP_WEAK,
	NO_OS_PULL_DOWN_WEAK
};

enum no_os_gpio_values {
	NO_OS_GPIO_LOW,
	NO_OS_GPIO_HIGH,
	NO_OS_GPIO_HIGH_Z
};

struct no_os_gpio_init_param {
	int32_t port;
	int32_t number;
	enum no_os_gpio_pull_up pull;
	const struct no_os_gpio_platform_ops *platform_ops;
	void *extra;
};

struct no_os_gpio_desc {
	int32_t port;
	int32_t number;
	enum no_os_gpio_pull_up pull;
	const struct no_os_gpio_platform_ops *platform_ops;
	void *extra;
};

struct no_os_gpio_platform_ops {
	int32_t (*gpio_ops_get)(struct no_os_gpio_desc **,
				const struct no_os_gpio_init_param *);
	int32_t (*gpio_ops_remove)(struct no_os_gpio_desc *);
	int32_t (*gpio_ops_direction_input)(struct no_os_gpio_desc *);
	int32_t (*gpio_ops_direction_output)(struct no_os_gpio_desc *, uint8_t);
	int32_t (*gpio_ops_get_direction)(struct no_os_gpio_desc *, uint8_t *);
	int32_t (*gpio_ops_set_value)(struct no_os_gpio_desc *, uint8_t);
	int32_t (*gpio_ops_get_value)(struct no_os_gpio_desc *, uint8_t *);
};

int32_t no_os_gpio_get(struct no_os_gpio_desc **desc,
		       const struct no_os_gpio_init_param *param);
int32_t no_os_gpio_remove(struct no_os_gpio_desc *desc);
int32_t no_os_gpio_direction_input(struct no_os_gpio_desc *desc);
int32_t no_os_gpio_direction_output(struct no_os_gpio_desc *desc, uint8_t value);
int32_t no_os_gpio_get_direction(struct no_os_gpio_desc *desc, uint8_t *direction);
int32_t no_os_gpio_set_value(struct no_os_gpio_desc *desc, uint8_t value);
int32_t no_os_gpio_get_value(struct no_os_gpio_desc *desc, uint8_t *value);

#endif /* _NO_OS_GPIO_H_ */
