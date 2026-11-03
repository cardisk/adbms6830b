/***************************************************************************//**
 *   @file   no_os_error.h
 *   @brief  Standard POSIX Error codes definition for no-OS drivers.
 *   @author DBogdan (dragos.bogdan@analog.com)
********************************************************************************
 * Copyright 2019(c) Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#ifndef _NO_OS_ERROR_H_
#define _NO_OS_ERROR_H_

#include <errno.h>

#ifndef __NO_OS_ELASTERROR
#define __NO_OS_ELASTERROR 2000
#endif

#define NO_OS_EOVERRUN	(__NO_OS_ELASTERROR + 1) /* Buffer overrun */

#define NO_OS_IS_ERR_VALUE(x)	((x) < 0)
#define NO_OS_IS_ERR(x)		(NO_OS_IS_ERR_VALUE((unsigned long)x))
#define NO_OS_PTR_ERR(x)	((long)x)
#define NO_OS_ERR_CAST(x)	((void *) x)

#endif /* _NO_OS_ERROR_H_ */
