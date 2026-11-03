/***************************************************************************//**
 *   @file   stm32_spi.h
 *   @brief  Header file for the STM32 SPI driver (Clean Standalone).
 *   @author Darius Berghe (darius.berghe@analog.com)
********************************************************************************
 * Copyright 2020(c) Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#ifndef _STM32_SPI_H_
#define _STM32_SPI_H_

#include <stdint.h>
#include "no_os_spi.h"
#include "no_os_gpio.h"
#include "stm32_hal.h"
#include "stm32_gpio.h"

struct stm32_spi_init_param {
	uint32_t chip_select_port;
	uint32_t (*get_input_clock)(void);
	uint32_t alternate;
};

struct stm32_spi_desc {
	SPI_HandleTypeDef *hspi;
	uint32_t input_clock;
	uint32_t alternate;
	struct no_os_gpio_desc *chip_select;
	struct stm32_gpio_init_param csip_extra;
	struct no_os_gpio_init_param csip;
};

extern const struct no_os_spi_platform_ops stm32_spi_ops;

int32_t stm32_spi_init(struct no_os_spi_desc **desc,
		       const struct no_os_spi_init_param *param);
int32_t stm32_spi_remove(struct no_os_spi_desc *desc);
int32_t stm32_spi_write_and_read(struct no_os_spi_desc *desc,
				 uint8_t *data,
				 uint16_t bytes_number);
int32_t stm32_spi_transfer(struct no_os_spi_desc *desc,
			   struct no_os_spi_msg *msgs,
			   uint32_t len);

#endif /* _STM32_SPI_H_ */
