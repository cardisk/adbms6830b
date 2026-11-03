/*******************************************************************************
 *   @file   no_os_alloc.c
 *   @brief  Implementation of no-OS memory allocation wrapper functions.
 *   @author GMois (george.mois@analog.com)
********************************************************************************
 * Copyright 2023(c) Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#include <stdlib.h>
#include "no_os_alloc.h"

void *no_os_malloc(size_t size)
{
	return malloc(size);
}

void *no_os_calloc(size_t nitems, size_t size)
{
	return calloc(nitems, size);
}

void no_os_free(void *ptr)
{
	free(ptr);
}
