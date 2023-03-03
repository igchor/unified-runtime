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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct uma_memory_tracker_t *uma_memory_tracker_handle_t;

enum uma_result_t umaMemoryTrackerCreate(uma_memory_tracker_handle_t *hTracker);
void umaMemoryTrackerDestroy(uma_memory_tracker_handle_t hTracker);

void *umaMemoryTrackerGetPool(uma_memory_tracker_handle_t hTracker, const void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* UMA_MEMORY_TRACKER_H */
