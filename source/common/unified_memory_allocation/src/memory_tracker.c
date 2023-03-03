/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <uma/memory_tracker.h>

#include "object.h"

#include <assert.h>
#include <stdlib.h>

UMA_DECLARE_TYPE(memory_tracker);

enum uma_result_t umaMemoryTrackerCreate(struct uma_memory_tracker_ops_t *ops, void *params,
                                         uma_memory_tracker_handle_t *hTracker) {
    UMA_CREATE_OBJECT(memory_tracker, ops, params, hTracker);
}

void umaMemoryTrackerDestroy(uma_memory_tracker_handle_t hTracker) {
    UMA_DESTROY_OBJECT(hTracker);
}

enum uma_result_t umaMemoryTrackerAdd(uma_memory_tracker_handle_t hTracker, void *pool, const void *ptr, size_t size) {
    return UMA_FORWARD_CALL(hTracker, add, pool, ptr, size);
}

enum uma_result_t umaMemoryTrackerRemove(uma_memory_tracker_handle_t hTracker, const void *ptr, size_t size) {
    return UMA_FORWARD_CALL(hTracker, remove, ptr, size);
}

void *umaMemoryTrackerFind(uma_memory_tracker_handle_t hTracker, const void *ptr) {
    return UMA_FORWARD_CALL(hTracker, find, ptr);
}
