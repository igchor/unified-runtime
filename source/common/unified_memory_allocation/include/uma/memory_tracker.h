/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UMA_MEMORY_TRACKER_H
#define UMA_MEMORY_TRACKER_H 1

#include <uma/base.h>
#include <uma/memory_tracker_ops.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct uma_memory_tracker_t *uma_memory_tracker_handle_t;

enum uma_result_t umaMemoryTrackerCreate(struct uma_memory_tracker_ops_t *ops, void *params, uma_memory_tracker_handle_t *hTracker);
void umaMemoryTrackerDestroy(uma_memory_tracker_handle_t tracker);

enum uma_result_t umaMemoryTrackerAdd(uma_memory_tracker_handle_t hTracker, void *pool, const void *ptr, size_t size);
enum uma_result_t umaMemoryTrackerRemove(uma_memory_tracker_handle_t hTracker, const void *ptr, size_t size);

void *umaMemoryTrackerFind(uma_memory_tracker_handle_t hTracker, const void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* UMA_MEMORY_TRACKER_H */
