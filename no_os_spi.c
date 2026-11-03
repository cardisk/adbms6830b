/***************************************************************************//**
 *   @file   no_os_spi.c
 *   @brief  Implementation of the SPI Interface dispatcher.
 *   @author Antoniu Miclaus (antoniu.miclaus@analog.com)
********************************************************************************
 * Copyright 2020(c) Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#include "no_os_spi.h"
#include "no_os_error.h"

int32_t no_os_spi_init(struct no_os_spi_desc **desc,
		       const struct no_os_spi_init_param *param)
{
	if (!desc || !param || !param->platform_ops || !param->platform_ops->init)
		return -EINVAL;

	return param->platform_ops->init(desc, param);
}

int32_t no_os_spi_remove(struct no_os_spi_desc *desc)
{
	if (!desc || !desc->platform_ops || !desc->platform_ops->remove)
		return -EINVAL;

	return desc->platform_ops->remove(desc);
}

int32_t no_os_spi_write_and_read(struct no_os_spi_desc *desc,
				 uint8_t *data,
				 uint16_t bytes_number)
{
	if (!desc || !desc->platform_ops || !desc->platform_ops->write_and_read)
		return -EINVAL;

	return desc->platform_ops->write_and_read(desc, data, bytes_number);
}

int32_t no_os_spi_transfer(struct no_os_spi_desc *desc,
			   struct no_os_spi_msg *msgs,
			   uint32_t len)
{
	if (!desc || !desc->platform_ops || !desc->platform_ops->transfer)
		return -EINVAL;

	return desc->platform_ops->transfer(desc, msgs, len);
}
