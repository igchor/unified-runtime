/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UMA_TEST_POOL_HPP
#define UMA_TEST_POOL_HPP 1

#include <malloc.h>
#include <uma/base.h>
#include <uma/memory_provider.h>

#include <gtest/gtest.h>
#include <stdlib.h>

#include "base.hpp"
#include "uma_helpers.hpp"

namespace uma_test {

auto wrapPoolUnique(uma_memory_pool_handle_t hPool) {
    return uma::pool_unique_handle_t(hPool, &umaPoolDestroy);
}

struct pool_base {
    uma_result_t initialize(struct uma_memory_provider_desc_t *, size_t) noexcept {
        return UMA_RESULT_SUCCESS;
    };
    void *malloc(size_t size) noexcept {
        return nullptr;
    }
    void *calloc(size_t, size_t) noexcept {
        return nullptr;
    }
    void *realloc(void *, size_t) noexcept {
        return nullptr;
    }
    void *aligned_malloc(size_t, size_t) noexcept {
        return nullptr;
    }
    size_t malloc_usable_size(void *) noexcept {
        return 0;
    }
    void free(void *) noexcept {
    }
    enum uma_result_t get_last_result(const char **ppMessage) noexcept {
        return UMA_RESULT_ERROR_UNKNOWN;
    }
};

struct malloc_pool : public pool_base {
    uma_result_t initialize(struct uma_memory_provider_desc_t *providers, size_t numProviders) noexcept {
        EXPECT_EQ(numProviders, 1);
        EXPECT_EQ(providers[0].providerType, UMA_MEMORY_PROVIDER_TYPE_TRACKER);

        this->tracker = providers[0].hProvider;
        return UMA_RESULT_SUCCESS;
    }
    void *malloc(size_t size) noexcept {
        auto *ptr = ::malloc(size);
        auto ret = umaMemoryProviderAlloc(tracker, size, 0, &ptr);
        EXPECT_EQ(ret, UMA_RESULT_SUCCESS);
        return ptr;
    }
    void *calloc(size_t num, size_t size) noexcept {
        auto *ptr = ::calloc(num, size);
        auto ret = umaMemoryProviderAlloc(tracker, num * size, 0, &ptr);
        EXPECT_EQ(ret, UMA_RESULT_SUCCESS);
        return ptr;
    }
    void *realloc(void *ptr, size_t size) noexcept {
        ptr = ::realloc(ptr, size);
        auto ret = umaMemoryProviderAlloc(tracker, size, 0, &ptr);
        EXPECT_EQ(ret, UMA_RESULT_SUCCESS);
        return ptr;
    }
    void *aligned_malloc(size_t size, size_t alignment) noexcept {
#ifdef _WIN32
        // we could use _aligned_alloc but it requires using _aligned_free...
        return nullptr;
#else
        auto *ptr = ::aligned_alloc(alignment, size);
        auto ret = umaMemoryProviderAlloc(tracker, size, 0, &ptr);
        EXPECT_EQ(ret, UMA_RESULT_SUCCESS);
        return ptr;
#endif
    }
    size_t malloc_usable_size(void *ptr) noexcept {
#ifdef _WIN32
        return _msize(ptr);
#else
        return ::malloc_usable_size(ptr);
#endif
    }
    void free(void *ptr) noexcept {
        auto ret = umaMemoryProviderFree(tracker, ptr, 0);
        EXPECT_EQ(ret, UMA_RESULT_SUCCESS);
        return ::free(ptr);
    }

    uma_memory_provider_handle_t tracker;
};

} // namespace uma_test

#endif /* UMA_TEST_POOL_HPP */
