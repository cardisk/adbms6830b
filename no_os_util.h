/***************************************************************************//**
 *   @file   no_os_util.h
 *   @brief  Header file of utility and bit-manipulation functions.
 *   @author DBogdan (dragos.bogdan@analog.com)
********************************************************************************
 * Copyright 2018(c) Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#ifndef _NO_OS_UTIL_H_
#define _NO_OS_UTIL_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define NO_OS_BIT(x)                    (1UL << (x))
#define NO_OS_BIT_ULL(x)                ((uint64_t)1ULL << (x))

#define NO_OS_ARRAY_SIZE(x)             (sizeof(x) / sizeof((x)[0]))

#define NO_OS_DIV_ROUND_UP(x, y)        (((x) + (y) - 1) / (y))
#define NO_OS_DIV_ROUND_CLOSEST(x, y)   (((x) + (y) / 2) / (y))

#define no_os_min(x, y)                 (((x) < (y)) ? (x) : (y))
#define no_os_max(x, y)                 (((x) > (y)) ? (x) : (y))
#define no_os_clamp(val, min_val, max_val) (no_os_max(no_os_min((val), (max_val)), (min_val)))

#define NO_OS_BITS_PER_LONG             32

#define NO_OS_GENMASK(h, l) ({                                  \
		uint32_t t = (uint32_t)(~0UL);                  \
		t = t << (NO_OS_BITS_PER_LONG - (h - l + 1));  \
		t = t >> (NO_OS_BITS_PER_LONG - (h + 1));      \
		t;                                              \
})

#define NO_OS_UNUSED_PARAM(x)           ((void)(x))

static inline uint32_t no_os_find_first_set_bit(uint32_t word)
{
	uint32_t first_set_bit = 0;
	if (!word)
		return 0;
	while (!(word & 0x1)) {
		word >>= 1;
		first_set_bit++;
	}
	return first_set_bit;
}

static inline uint32_t no_os_field_prep(uint32_t mask, uint32_t val)
{
	return (val << no_os_find_first_set_bit(mask)) & mask;
}

static inline uint32_t no_os_field_get(uint32_t mask, uint32_t word)
{
	return (word & mask) >> no_os_find_first_set_bit(mask);
}

#endif /* _NO_OS_UTIL_H_ */
