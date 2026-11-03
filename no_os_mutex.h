/*******************************************************************************
 *   @file   no_os_mutex.h
 *   @brief  Header file of minimal standalone mutex stubs.
 *   @author Robert Budai (robert.budai@analog.com)
********************************************************************************
 * Copyright 2023(c) Analog Devices, Inc.
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#ifndef _NO_OS_MUTEX_H_
#define _NO_OS_MUTEX_H_

static inline void no_os_mutex_init(void **mutex) { (void)mutex; }
static inline void no_os_mutex_lock(void *mutex) { (void)mutex; }
static inline void no_os_mutex_unlock(void *mutex) { (void)mutex; }
static inline void no_os_mutex_remove(void *mutex) { (void)mutex; }

#endif /* _NO_OS_MUTEX_H_ */
