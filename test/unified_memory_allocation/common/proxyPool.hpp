/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UR_UMA_PROXY_POOL_HPP
#define UR_UMA_PROXY_POOL_HPP 1

#include <uma/base.h>
#include <uma/memory_provider.h>
#include <uma/memory_tracker.h>

#include "helpers.h"

namespace uma_test {

struct proxy_pool {
    proxy_pool(uma_memory_provider_handle_t hProvider, uma_memory_tracker_handle_t hTracker, void *pool) {
        auto ret = umaTrackingMemoryProviderCreate(hProvider, hTracker, pool, &this->hProvider);
        EXPECT_EQ(ret, UMA_RESULT_SUCCESS);
    }

    void *malloc(size_t size) {
        return aligned_malloc(size, 0);
    }

    void *calloc(size_t, size_t) {
        return nullptr;
    }

    void *realloc(void *, size_t) {
        return nullptr;
    }

    void *aligned_malloc(size_t size, size_t alignment) {
        void *ptr;

        auto ret = umaMemoryProviderAlloc(hProvider, size, alignment, &ptr);
        EXPECT_EQ(ret, UMA_RESULT_SUCCESS);

        return ptr;
    }

    size_t malloc_usable_size(void *) {
        return 0;
    }

    void free(void *ptr) {
        EXPECT_EQ(umaMemoryProviderFree(hProvider, ptr, 0), UMA_RESULT_SUCCESS);
    }

    enum uma_result_t get_last_result(const char **ppMessage) {
        return UMA_RESULT_ERROR_UNKNOWN;
    }

  private:
    uma_memory_provider_handle_t hProvider;
};

} // namespace uma_test

#endif /* UR_UMA_PROXY_POOL_HPP */
