/*******************************************************************************
 *   @file   no_os_alloc.h
 *   @brief  Header file of memory allocator wrapper.
 *   @author GMois (george.mois@analog.com)
********************************************************************************
 * Copyright 2023(c) Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#ifndef _NO_OS_ALLOC_H_
#define _NO_OS_ALLOC_H_

#include <stdlib.h>

void *no_os_malloc(size_t size);
void *no_os_calloc(size_t nitems, size_t size);
void no_os_free(void *ptr);

#endif /* _NO_OS_ALLOC_H_ */
