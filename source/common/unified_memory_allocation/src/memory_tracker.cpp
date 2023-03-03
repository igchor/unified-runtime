/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "memory_tracker_internal.h"
#include <uma/memory_provider.h>
#include <uma/memory_provider_ops.h>

#include <map>
#include <mutex>
#include <shared_mutex>

// TODO: reimplement in C and optimize...
struct uma_memory_tracker_t {
    enum uma_result_t add(void *pool, const void *ptr, size_t size) {
        std::unique_lock<std::shared_mutex> lock(mtx);

        auto ret = map.try_emplace(reinterpret_cast<uintptr_t>(ptr), size, pool);
        return ret.second ? UMA_RESULT_SUCCESS : UMA_RESULT_ERROR_UNKNOWN;
    }

    enum uma_result_t remove(const void *ptr, size_t size) {
        std::unique_lock<std::shared_mutex> lock(mtx);

        map.erase(reinterpret_cast<uintptr_t>(ptr));

        // TODO: handle removing part of the range
        (void)size;

        return UMA_RESULT_SUCCESS;
    }

    void *find(const void *ptr) {
        std::shared_lock<std::shared_mutex> lock(mtx);

        auto intptr = reinterpret_cast<uintptr_t>(ptr);
        auto it = map.upper_bound(intptr);
        if (it == map.begin()) {
            return nullptr;
        }

        --it;
        if (intptr >= it->first && intptr < it->first + it->second.first) {
            return it->second.second;
        }

        return nullptr;
    }

  private:
    std::shared_mutex mtx;
    std::map<uintptr_t, std::pair<size_t, void *>> map;
};

extern "C" {

uma_memory_tracker_handle_t umaMemoryTrackerGet() {
    static uma_memory_tracker_t tracker;
    return &tracker;
}

void *umaMemoryTrackerGetPool(uma_memory_tracker_handle_t hTracker, const void *ptr) {
    return hTracker->find(ptr);
}

enum uma_result_t umaMemoryTrackerAdd(uma_memory_tracker_handle_t hTracker, void *pool, const void *ptr, size_t size) {
    return hTracker->add(pool, ptr, size);
}

enum uma_result_t umaMemoryTrackerRemove(uma_memory_tracker_handle_t hTracker, const void *ptr, size_t size) {
    return hTracker->remove(ptr, size);
}

struct uma_tracking_memory_provider_t {
    uma_memory_provider_handle_t hUpstream;
    uma_memory_tracker_handle_t hTracker;
    uma_memory_pool_handle_t pool;
};

typedef struct uma_tracking_memory_provider_t uma_tracking_memory_provider_t;

static enum uma_result_t trackingAlloc(void *hProvider,
                                       size_t size, size_t alignment,
                                       void **ptr) {
    uma_tracking_memory_provider_t *p = (uma_tracking_memory_provider_t *)hProvider;
    enum uma_result_t ret = UMA_RESULT_SUCCESS;

    if (p->hUpstream) {
        ret = umaMemoryProviderAlloc(p->hUpstream, size, alignment, ptr);
        if (ret != UMA_RESULT_SUCCESS) {
            return ret;
        }
    } else {
        // expects *ptr to be a valid address
    }

    // TODO: is it fine that allocation and adding memory to tracker is not a single atomic action
    ret = umaMemoryTrackerAdd(p->hTracker, p->pool, *ptr, size);
    if (ret != UMA_RESULT_SUCCESS && p->hUpstream) {
        if (umaMemoryTrackerRemove(p->hTracker, *ptr, size)) {
            // TODO: LOG
        }
    }

    return ret;
}

static enum uma_result_t trackingFree(void *hProvider,
                                      void *ptr, size_t size) {
    enum uma_result_t ret;
    uma_tracking_memory_provider_t *p = (uma_tracking_memory_provider_t *)hProvider;

    ret = umaMemoryTrackerRemove(p->hTracker, ptr, size);
    if (ret != UMA_RESULT_SUCCESS) {
        return ret;
    }

    if (p->hUpstream) {
        // TODO: is it fine that deallocation and removing memory from the tracker is not a single atomic action
        ret = umaMemoryProviderFree(p->hUpstream, ptr, size);
        if (ret != UMA_RESULT_SUCCESS) {
            if (umaMemoryTrackerAdd(p->hTracker, p->pool, ptr, size) != UMA_RESULT_SUCCESS) {
                // TODO: LOG
            }
            return ret;
        }
    }

    return ret;
}

static enum uma_result_t trackingInitialize(void *params, void **ret) {
    uma_tracking_memory_provider_t *provider =
        (uma_tracking_memory_provider_t *)malloc(sizeof(uma_tracking_memory_provider_t));
    if (!provider) {
        return UMA_RESULT_ERROR_OUT_OF_HOST_MEMORY;
    }

    *provider = *((uma_tracking_memory_provider_t *)params);
    *ret = provider;
    return UMA_RESULT_SUCCESS;
}

static void trackingFinalize(void *provider) {
    free(provider);
}

enum uma_result_t umaTrackingMemoryProviderCreate(uma_memory_provider_handle_t hUpstream,
                                                  uma_memory_pool_handle_t hPool, uma_memory_provider_handle_t *hTrackingProvider) {
    uma_tracking_memory_provider_t params;
    params.hUpstream = hUpstream,
    params.hTracker = umaMemoryTrackerGet(),
    params.pool = hPool;

    struct uma_memory_provider_ops_t trackingMemoryProviderOps;
    trackingMemoryProviderOps.version = UMA_VERSION_CURRENT;
    trackingMemoryProviderOps.initialize = trackingInitialize;
    trackingMemoryProviderOps.finalize = trackingFinalize;
    trackingMemoryProviderOps.alloc = trackingAlloc;
    trackingMemoryProviderOps.free = trackingFree;
    trackingMemoryProviderOps.get_last_result = NULL;

    return umaMemoryProviderCreate(&trackingMemoryProviderOps, &params,
                                   hTrackingProvider);
}
}
