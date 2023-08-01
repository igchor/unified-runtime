/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
 * See LICENSE.TXT
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 */

#include "memory_pool_internal.hpp"
#include "memory_provider_internal.h"
#include "memory_tracker.h"

#include <umf/memory_pool.h>

#include <assert.h>
#include <stdlib.h>

static void
destroyMemoryProviderWrappers(umf_memory_provider_handle_t *providers,
                              size_t numProviders) {
    for (size_t i = 0; i < numProviders; i++) {
        umfMemoryProviderDestroy(providers[i]);
    }

    free(providers);
}

umf_memory_pool_t::umf_memory_pool_t(const struct umf_memory_pool_ops_t *ops,
                                umf_memory_provider_handle_t *providers,
                                size_t numProviders, void *params) {
    if (!numProviders || !providers) {
        throw UMF_RESULT_ERROR_INVALID_ARGUMENT;
    }

    size_t providerInd = 0;
    assert(ops->version == UMF_VERSION_CURRENT);

    impl_ = std::make_unique<impl>();
    impl_->providers = (umf_memory_provider_handle_t*)
        calloc(numProviders, sizeof(umf_memory_provider_handle_t));
    if (!impl_->providers) {
        throw UMF_RESULT_ERROR_OUT_OF_HOST_MEMORY;
    }


    impl_->numProviders = numProviders;

    // Wrap each provider with memory tracking provider.
    for (providerInd = 0; providerInd < numProviders; providerInd++) {
        auto ret = umfTrackingMemoryProviderCreate(providers[providerInd], this,
                                              &impl_->providers[providerInd]);
        if (ret != UMF_RESULT_SUCCESS) {
            throw ret;
        }
    }

    impl_->ops = *ops;
    auto ret = ops->initialize(impl_->providers, impl_->numProviders, params,
                          &impl_->pool_priv);
    if (ret != UMF_RESULT_SUCCESS) {
        throw ret;
    }
}

void umfPoolDestroy(umf_memory_pool_handle_t hPool) {
    hPool->impl_->ops.finalize(hPool->impl_->pool_priv);
    destroyMemoryProviderWrappers(hPool->impl_->providers, hPool->impl_->numProviders);
    delete hPool;
}

enum umf_result_t umfFree(void *ptr) {
    umf_memory_pool_handle_t hPool = umfPoolByPtr(ptr);
    if (hPool) {
        return umfPoolFree(hPool, ptr);
    }
    return UMF_RESULT_SUCCESS;
}

umf_memory_pool_handle_t umfPoolByPtr(const void *ptr) {
    return (umf_memory_pool_handle_t) umfMemoryTrackerGetPool(umfMemoryTrackerGet(), ptr);
}

enum umf_result_t
umfPoolGetMemoryProviders(umf_memory_pool_handle_t hPool, size_t numProviders,
                          umf_memory_provider_handle_t *hProviders,
                          size_t *numProvidersRet) {
    if (hProviders && numProviders < hPool->impl_->numProviders) {
        return UMF_RESULT_ERROR_INVALID_ARGUMENT;
    }

    if (numProvidersRet) {
        *numProvidersRet = hPool->impl_->numProviders;
    }

    if (hProviders) {
        for (size_t i = 0; i < hPool->impl_->numProviders; i++) {
            umfTrackingMemoryProviderGetUpstreamProvider(
                (umf_memory_provider_handle_t) umfMemoryProviderGetPriv(hPool->impl_->providers[i]), hProviders + i);
        }
    }

    return UMF_RESULT_SUCCESS;
}
