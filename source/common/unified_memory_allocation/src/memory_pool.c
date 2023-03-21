/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "memory_tracker_internal.h"
#include <uma/memory_pool.h>
#include <uma/memory_pool_ops.h>

#include <assert.h>
#include <stdlib.h>

struct uma_memory_pool_t {
    void *pool_priv;
    struct uma_memory_pool_ops_t ops;

    // Holds array of memory providers. All 'data' memory providers are wrapped
    // by memory tracking providers (owned and released by UMA). All 'metadata'
    // providers are stored as they were passed.
    struct uma_memory_provider_desc_t *providers;

    size_t numProviders;
};

static void destroyMemoryProviders(struct uma_memory_provider_desc_t *providers,
                                   size_t numProviders) {
    for (size_t i = 0; i < numProviders; i++) {
        if (providers[i].providerType == UMA_MEMORY_PROVIDER_TYPE_DATA) {
            umaMemoryProviderDestroy(providers[i].hProvider);
        } else {
            // 'metadata' providers are owned by the user
        }
    }

    free(providers);
}

enum uma_result_t umaPoolCreate(struct uma_memory_pool_ops_t *ops,
                                struct uma_memory_provider_desc_t *providers,
                                size_t numProviders, void *params,
                                uma_memory_pool_handle_t *hPool) {
    if (!numProviders || !providers) {
        return UMA_RESULT_ERROR_INVALID_ARGUMENT;
    }

    int dataProviderPresent = 0;
    for (size_t i = 0; i < numProviders; i++) {
        if (providers[i].providerType == UMA_MEMORY_PROVIDER_TYPE_DATA) {
            dataProviderPresent = 1;
            break;
        }
    }

    if (!dataProviderPresent) {
        return UMA_RESULT_ERROR_INVALID_ARGUMENT;
    }

    enum uma_result_t ret = UMA_RESULT_SUCCESS;
    uma_memory_pool_handle_t pool = malloc(sizeof(struct uma_memory_pool_t));
    if (!pool) {
        return UMA_RESULT_ERROR_OUT_OF_HOST_MEMORY;
    }

    assert(ops->version == UMA_VERSION_CURRENT);

    pool->providers =
        calloc(numProviders, sizeof(struct uma_memory_provider_desc_t));
    if (!pool->providers) {
        ret = UMA_RESULT_ERROR_OUT_OF_HOST_MEMORY;
        goto err_providers_alloc;
    }

    size_t providerInd = 0;
    pool->numProviders = numProviders;

    // Wrap each 'data' provider with memory tracking provider and store 'metadata' providers.
    for (providerInd = 0; providerInd < numProviders; providerInd++) {
        pool->providers[providerInd].providerType =
            providers[providerInd].providerType;
        if (providers[providerInd].providerType ==
            UMA_MEMORY_PROVIDER_TYPE_DATA) {
            ret = umaTrackingMemoryProviderCreate(
                providers[providerInd].hProvider, pool,
                &pool->providers[providerInd].hProvider);
            if (ret != UMA_RESULT_SUCCESS) {
                goto err_providers_init;
            }
        } else {
            pool->providers[providerInd].hProvider =
                providers[providerInd].hProvider;
        }
    }

    pool->ops = *ops;
    ret = ops->initialize(pool->providers, pool->numProviders, params,
                          &pool->pool_priv);
    if (ret != UMA_RESULT_SUCCESS) {
        goto err_pool_init;
    }

    *hPool = pool;
    return UMA_RESULT_SUCCESS;

err_pool_init:
err_providers_init:
    destroyMemoryProviders(pool->providers, providerInd);
err_providers_alloc:
    free(pool);

    return ret;
}

void umaPoolDestroy(uma_memory_pool_handle_t hPool) {
    hPool->ops.finalize(hPool->pool_priv);
    destroyMemoryProviders(hPool->providers, hPool->numProviders);
    free(hPool);
}

void *umaPoolMalloc(uma_memory_pool_handle_t hPool, size_t size) {
    return hPool->ops.malloc(hPool->pool_priv, size);
}

void *umaPoolAlignedMalloc(uma_memory_pool_handle_t hPool, size_t size,
                           size_t alignment) {
    return hPool->ops.aligned_malloc(hPool->pool_priv, size, alignment);
}

void *umaPoolCalloc(uma_memory_pool_handle_t hPool, size_t num, size_t size) {
    return hPool->ops.calloc(hPool->pool_priv, num, size);
}

void *umaPoolRealloc(uma_memory_pool_handle_t hPool, void *ptr, size_t size) {
    return hPool->ops.realloc(hPool->pool_priv, ptr, size);
}

size_t umaPoolMallocUsableSize(uma_memory_pool_handle_t hPool, void *ptr) {
    return hPool->ops.malloc_usable_size(hPool->pool_priv, ptr);
}

void umaPoolFree(uma_memory_pool_handle_t hPool, void *ptr) {
    hPool->ops.free(hPool->pool_priv, ptr);
}

void umaFree(void *ptr) {
    uma_memory_pool_handle_t hPool = umaPoolByPtr(ptr);
    if (hPool) {
        umaPoolFree(hPool, ptr);
    }
}

enum uma_result_t umaPoolGetLastResult(uma_memory_pool_handle_t hPool,
                                       const char **ppMessage) {
    return hPool->ops.get_last_result(hPool->pool_priv, ppMessage);
}

uma_memory_pool_handle_t umaPoolByPtr(const void *ptr) {
    return umaMemoryTrackerGetPool(umaMemoryTrackerGet(), ptr);
}
