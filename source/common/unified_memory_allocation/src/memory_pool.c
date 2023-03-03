/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "memory_tracker_internal.h"
#include <uma/memory_pool.h>

#include <assert.h>
#include <stdlib.h>

struct uma_memory_pool_t {
    void *priv; // so that &pool->priv == pool
    struct uma_memory_pool_ops_t ops;
};

enum uma_result_t umaPoolCreate(struct uma_memory_pool_ops_t *ops, void *params,
                                uma_memory_pool_handle_t *hPool) {
    uma_memory_pool_handle_t pool = malloc(sizeof(struct uma_memory_pool_t));
    if (!pool) {
        return UMA_RESULT_ERROR_OUT_OF_HOST_MEMORY;
    }

    assert(ops->version == UMA_VERSION_CURRENT);

    pool->ops = *ops;
    enum uma_result_t ret = ops->initialize(params, &pool->priv);
    if (ret != UMA_RESULT_SUCCESS) {
        return ret;
    }

    *hPool = pool;

    return UMA_RESULT_SUCCESS;
}

void umaPoolDestroy(uma_memory_pool_handle_t hPool) {
    hPool->ops.finalize(hPool->priv);
    free(hPool);
}

void *umaPoolMalloc(uma_memory_pool_handle_t hPool, size_t size) {
    return hPool->ops.malloc(hPool->priv, size);
}

void *umaPoolAlignedMalloc(uma_memory_pool_handle_t hPool, size_t size,
                           size_t alignment) {
    return hPool->ops.aligned_malloc(hPool->priv, size, alignment);
}

void *umaPoolCalloc(uma_memory_pool_handle_t hPool, size_t num, size_t size) {
    return hPool->ops.calloc(hPool->priv, num, size);
}

void *umaPoolRealloc(uma_memory_pool_handle_t hPool, void *ptr, size_t size) {
    return hPool->ops.realloc(hPool->priv, ptr, size);
}

size_t umaPoolMallocUsableSize(uma_memory_pool_handle_t hPool, void *ptr) {
    return hPool->ops.malloc_usable_size(hPool->priv, ptr);
}

void umaPoolFree(uma_memory_pool_handle_t hPool, void *ptr) {
    hPool->ops.free(hPool->priv, ptr);
}

enum uma_result_t
umaPoolGetLastResult(uma_memory_pool_handle_t hPool, const char **ppMessage) {
    return hPool->ops.get_last_result(hPool->priv, ppMessage);
}

uma_memory_pool_handle_t umaPoolByPtr(const void *ptr) {
    return umaMemoryTrackerGetPool(umaMemoryTrackerGet(), ptr);
}
