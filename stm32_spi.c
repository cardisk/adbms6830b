/***************************************************************************//**
 *   @file   stm32_spi.c
 *   @brief  Implementation of STM32 SPI driver (Clean Standalone).
 *   @author Darius Berghe (darius.berghe@analog.com)
********************************************************************************
 * Copyright 2020(c) Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#include <stdlib.h>
#include "no_os_util.h"
#include "no_os_error.h"
#include "no_os_alloc.h"
#include "no_os_delay.h"
#include "stm32_spi.h"

extern SPI_HandleTypeDef hspi1;
#ifdef SPI2
extern SPI_HandleTypeDef hspi2;
#endif
#ifdef SPI3
extern SPI_HandleTypeDef hspi3;
#endif
#ifdef SPI4
extern SPI_HandleTypeDef hspi4;
#endif
#ifdef SPI5
extern SPI_HandleTypeDef hspi5;
#endif

static SPI_HandleTypeDef *const stm32_hspi_instances[] = {
	NULL,       /* 0: Unused */
	&hspi1,     /* 1: SPI1 */
#ifdef SPI2
	&hspi2,     /* 2: SPI2 */
#else
	NULL,
#endif
#ifdef SPI3
	&hspi3,     /* 3: SPI3 */
#else
	NULL,
#endif
#ifdef SPI4
	&hspi4,     /* 4: SPI4 */
#else
	NULL,
#endif
#ifdef SPI5
	&hspi5,     /* 5: SPI5 */
#else
	NULL,
#endif
};

int32_t stm32_spi_init(struct no_os_spi_desc **desc,
		       const struct no_os_spi_init_param *param)
{
	struct no_os_spi_desc *spi_desc;
	struct stm32_spi_desc *sdesc;
	const struct stm32_spi_init_param *sinit;
	int32_t ret;

	if (!desc || !param || !param->extra)
		return -EINVAL;

	if (param->device_id >= NO_OS_ARRAY_SIZE(stm32_hspi_instances) ||
	    !stm32_hspi_instances[param->device_id])
		return -EINVAL;

	sinit = param->extra;

	spi_desc = (struct no_os_spi_desc *)no_os_calloc(1, sizeof(*spi_desc));
	if (!spi_desc)
		return -ENOMEM;

	sdesc = (struct stm32_spi_desc *)no_os_calloc(1, sizeof(*sdesc));
	if (!sdesc) {
		no_os_free(spi_desc);
		return -ENOMEM;
	}

	spi_desc->device_id = param->device_id;
	spi_desc->max_speed_hz = param->max_speed_hz;
	spi_desc->chip_select = param->chip_select;
	spi_desc->mode = param->mode;
	spi_desc->bit_order = param->bit_order;
	spi_desc->platform_ops = param->platform_ops;
	spi_desc->extra = sdesc;

	sdesc->hspi = stm32_hspi_instances[param->device_id];
	sdesc->alternate = sinit->alternate;
	if (sinit->get_input_clock)
		sdesc->input_clock = sinit->get_input_clock();

	/* Configure CS GPIO Pin */
	sdesc->csip_extra.mode = GPIO_MODE_OUTPUT_PP;
	sdesc->csip_extra.speed = GPIO_SPEED_FREQ_HIGH;
	sdesc->csip.port = sinit->chip_select_port;
	sdesc->csip.number = param->chip_select;
	sdesc->csip.pull = NO_OS_PULL_NONE;
	sdesc->csip.platform_ops = &stm32_gpio_ops;
	sdesc->csip.extra = &sdesc->csip_extra;

	ret = no_os_gpio_get(&sdesc->chip_select, &sdesc->csip);
	if (ret) {
		no_os_free(sdesc);
		no_os_free(spi_desc);
		return ret;
	}

	/* Ensure CS starts deasserted (HIGH) */
	no_os_gpio_direction_output(sdesc->chip_select, NO_OS_GPIO_HIGH);

	*desc = spi_desc;
	return 0;
}

int32_t stm32_spi_remove(struct no_os_spi_desc *desc)
{
	struct stm32_spi_desc *sdesc;

	if (!desc)
		return -EINVAL;

	sdesc = desc->extra;
	if (sdesc) {
		if (sdesc->chip_select)
			no_os_gpio_remove(sdesc->chip_select);
		no_os_free(sdesc);
	}
	no_os_free(desc);

	return 0;
}

int32_t stm32_spi_transfer(struct no_os_spi_desc *desc,
			   struct no_os_spi_msg *msgs,
			   uint32_t len)
{
	struct stm32_spi_desc *sdesc;
	HAL_StatusTypeDef hal_status;

	if (!desc || !desc->extra || !msgs)
		return -EINVAL;

	sdesc = desc->extra;

	for (uint32_t i = 0; i < len; i++) {
		if (!msgs[i].tx_buff && !msgs[i].rx_buff)
			return -EINVAL;

		/* Assert CS (Active-Low) */
		no_os_gpio_set_value(sdesc->chip_select, NO_OS_GPIO_LOW);

		if (msgs[i].cs_delay_first)
			no_os_udelay(msgs[i].cs_delay_first);

		if (msgs[i].tx_buff && msgs[i].rx_buff) {
			hal_status = HAL_SPI_TransmitReceive(sdesc->hspi,
							     msgs[i].tx_buff,
							     msgs[i].rx_buff,
							     msgs[i].bytes_number,
							     HAL_MAX_DELAY);
		} else if (msgs[i].tx_buff) {
			hal_status = HAL_SPI_Transmit(sdesc->hspi,
						      msgs[i].tx_buff,
						      msgs[i].bytes_number,
						      HAL_MAX_DELAY);
		} else {
			hal_status = HAL_SPI_Receive(sdesc->hspi,
						     msgs[i].rx_buff,
						     msgs[i].bytes_number,
						     HAL_MAX_DELAY);
		}

		if (msgs[i].cs_delay_last)
			no_os_udelay(msgs[i].cs_delay_last);

		if (msgs[i].cs_change)
			no_os_gpio_set_value(sdesc->chip_select, NO_OS_GPIO_HIGH);

		if (msgs[i].cs_change_delay)
			no_os_udelay(msgs[i].cs_change_delay);

		if (hal_status != HAL_OK)
			return (hal_status == HAL_TIMEOUT) ? -ETIMEDOUT : -EIO;
	}

	return 0;
}

int32_t stm32_spi_write_and_read(struct no_os_spi_desc *desc,
				 uint8_t *data,
				 uint16_t bytes_number)
{
	struct no_os_spi_msg msg = {
		.bytes_number = bytes_number,
		.cs_change = true,
		.rx_buff = data,
		.tx_buff = data,
	};

	if (!desc || !desc->extra || !data)
		return -EINVAL;

	if (!bytes_number)
		return 0;

	return stm32_spi_transfer(desc, &msg, 1);
}

const struct no_os_spi_platform_ops stm32_spi_ops = {
	.init = stm32_spi_init,
	.write_and_read = stm32_spi_write_and_read,
	.transfer = stm32_spi_transfer,
	.remove = stm32_spi_remove,
};
