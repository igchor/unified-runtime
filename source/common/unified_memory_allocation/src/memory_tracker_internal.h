/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UMA_MEMORY_TRACKER_INTERNAL_H
#define UMA_MEMORY_TRACKER_INTERNAL_H 1

#include <uma/base.h>
#include <uma/memory_pool.h>
#include <uma/memory_provider.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct uma_memory_tracker_t *uma_memory_tracker_handle_t;

uma_memory_tracker_handle_t umaMemoryTrackerGet();
enum uma_result_t umaTrackingMemoryProviderCreate(uma_memory_provider_handle_t hUpstream,
                                                  uma_memory_pool_handle_t hPool, uma_memory_provider_handle_t *hTrackingProvider);
enum uma_result_t umaMemoryTrackerAdd(uma_memory_tracker_handle_t hTracker, void *pool, const void *ptr, size_t size);
enum uma_result_t umaMemoryTrackerRemove(uma_memory_tracker_handle_t hTracker, const void *ptr, size_t size);
void *umaMemoryTrackerGetPool(uma_memory_tracker_handle_t hTracker, const void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* UMA_MEMORY_TRACKER_INTERNAL_H */
