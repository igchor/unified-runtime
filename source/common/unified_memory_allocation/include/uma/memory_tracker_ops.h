/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UMA_MEMORY_TRACKER_OPS_H
#define UMA_MEMORY_TRACKER_OPS_H 1

#include <uma/base.h>

#ifdef __cplusplus
extern "C" {
#endif

/// This structure comprises function pointers used by corresponding
/// umaMemoryTracker* calls. Each memory tracker implementation should
/// initialize all function pointers.
struct uma_memory_tracker_ops_t {
    /// Version of the ops structure.
    /// Should be initialized using UMA_VERSION_CURRENT
    uint32_t version;

    ///
    /// \brief Intializes memory tracker
    /// \param params tracker-specific params
    /// \param tracker returns pointer to the tracker
    /// \return UMA_RESULT_SUCCESS on success or appropriate error code on
    /// failure
    enum uma_result_t (*initialize)(void *params, void **tracker);

    ///
    /// \brief Finalizes memory tracker
    /// \param tracker tracker to finalize
    void (*finalize)(void *tracker);

    /// Refer to memory_tracker.h for description of those functions
    enum uma_result_t (*add)(void *tracker, void *pool, const void *ptr, size_t size);
    enum uma_result_t (*remove)(void *tracker, const void *ptr, size_t size);
    void *(*find)(void *tracker, const void *ptr);
    // TODO: get_last_result
};

#ifdef __cplusplus
}
#endif

#endif /* UMA_MEMORY_TRACKER_OPS_H */
