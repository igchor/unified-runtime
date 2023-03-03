/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <uma/memory_provider.h>

#include <assert.h>
#include <stdlib.h>

struct uma_memory_provider_t {
    struct uma_memory_provider_ops_t ops;
    void *priv;
};

enum uma_result_t
umaMemoryProviderCreate(struct uma_memory_provider_ops_t *ops, void *params,
                        uma_memory_provider_handle_t *hProvider) {
    uma_memory_provider_handle_t provider =
        malloc(sizeof(struct uma_memory_provider_t));
    if (!provider) {
        return UMA_RESULT_ERROR_OUT_OF_HOST_MEMORY;
    }

    assert(ops->version == UMA_VERSION_CURRENT);

    provider->ops = *ops;

    void *priv;
    enum uma_result_t ret = ops->initialize(params, &priv);
    if (ret != UMA_RESULT_SUCCESS) {
        return ret;
    }

    provider->priv = priv;

    *hProvider = provider;

    return UMA_RESULT_SUCCESS;
}

void umaMemoryProviderDestroy(uma_memory_provider_handle_t hProvider) {
    hProvider->ops.finalize(hProvider->priv);
    free(hProvider);
}

enum uma_result_t umaMemoryProviderAlloc(uma_memory_provider_handle_t hProvider,
                                         size_t size, size_t alignment,
                                         void **ptr) {
    return hProvider->ops.alloc(hProvider->priv, size, alignment, ptr);
}

enum uma_result_t umaMemoryProviderFree(uma_memory_provider_handle_t hProvider,
                                        void *ptr, size_t size) {
    return hProvider->ops.free(hProvider->priv, ptr, size);
}

enum uma_result_t
umaMemoryProviderGetLastResult(uma_memory_provider_handle_t hProvider, const char **ppMessage) {
    return hProvider->ops.get_last_result(hProvider->priv, ppMessage);
}

struct uma_tracking_memory_provider_t {
    uma_memory_provider_handle_t hUpstream;
    uma_memory_tracker_handle_t hTracker;
    void *pool;
};

typedef struct uma_tracking_memory_provider_t uma_tracking_memory_provider_t;

static enum uma_result_t trackingAlloc(void *hProvider,
                                       size_t size, size_t alignment,
                                       void **ptr) {
    uma_tracking_memory_provider_t *p = (uma_tracking_memory_provider_t *)hProvider;

    enum uma_result_t ret = umaMemoryProviderAlloc(p->hUpstream, size, alignment, ptr);
    if (ret != UMA_RESULT_SUCCESS) {
        return ret;
    }

    ret = umaMemoryTrackerAdd(p->hTracker, p->pool, *ptr, size);
    if (ret != UMA_RESULT_SUCCESS) {
        // TODO: free the allocation and return error?
    }

    return ret;
}

static enum uma_result_t trackingFree(void *hProvider,
                                      void *ptr, size_t size) {
    uma_tracking_memory_provider_t *p = (uma_tracking_memory_provider_t *)hProvider;

    enum uma_result_t ret = umaMemoryProviderFree(p->hUpstream, ptr, size);
    if (ret != UMA_RESULT_SUCCESS) {
        return ret;
    }

    ret = umaMemoryTrackerRemove(p->hTracker, ptr, size);
    if (ret != UMA_RESULT_SUCCESS) {
        // TODO
    }

    return ret;
}

static enum uma_result_t trackingInitialize(void *params, void **ret) {
    uma_tracking_memory_provider_t *provider = (uma_tracking_memory_provider_t *)malloc(sizeof(uma_tracking_memory_provider_t));
    *provider = *((uma_tracking_memory_provider_t *)params);
    *ret = provider;
    return UMA_RESULT_SUCCESS;
}

static void trackingFinalize(void *provider) {
    free(provider);
}

static struct uma_memory_provider_ops_t umaTrackingMemoryProviderOps() {
    struct uma_memory_provider_ops_t ops = {
        .version = UMA_VERSION_CURRENT,
        .initialize = trackingInitialize,
        .finalize = trackingFinalize,
        .alloc = trackingAlloc,
        .free = trackingFree,
        .get_last_result = NULL, // TODO
    };

    return ops;
}

enum uma_result_t umaTrackingMemoryProviderCreate(uma_memory_provider_handle_t hUpstream,
                                                  uma_memory_tracker_handle_t hTracker,
                                                  void *pool, uma_memory_provider_handle_t *hTrackingProvider) {
    uma_tracking_memory_provider_t params = {
        .hUpstream = hUpstream,
        .hTracker = hTracker,
        .pool = pool};

    struct uma_memory_provider_ops_t ops = umaTrackingMemoryProviderOps();
    return umaMemoryProviderCreate(&ops, &params,
                                   hTrackingProvider);
}
