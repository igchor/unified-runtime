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

#include <umf/memory_pool.h>
#include <umf/memory_pool_ops.h>

#include <assert.h>
#include <stdlib.h>

void *umfPoolMalloc(umf_memory_pool_handle_t hPool, size_t size) {
    return hPool->impl_->ops.malloc(hPool->impl_->pool_priv, size);
}

void *umfPoolAlignedMalloc(umf_memory_pool_handle_t hPool, size_t size,
                           size_t alignment) {
    return hPool->impl_->ops.aligned_malloc(hPool->impl_->pool_priv, size, alignment);
}

void *umfPoolCalloc(umf_memory_pool_handle_t hPool, size_t num, size_t size) {
    return hPool->impl_->ops.calloc(hPool->impl_->pool_priv, num, size);
}

void *umfPoolRealloc(umf_memory_pool_handle_t hPool, void *ptr, size_t size) {
    return hPool->impl_->ops.realloc(hPool->impl_->pool_priv, ptr, size);
}

size_t umfPoolMallocUsableSize(umf_memory_pool_handle_t hPool, void *ptr) {
    return hPool->impl_->ops.malloc_usable_size(hPool->impl_->pool_priv, ptr);
}

enum umf_result_t umfPoolFree(umf_memory_pool_handle_t hPool, void *ptr) {
    return hPool->impl_->ops.free(hPool->impl_->pool_priv, ptr);
}

enum umf_result_t
umfPoolGetLastAllocationError(umf_memory_pool_handle_t hPool) {
    return hPool->impl_->ops.get_last_allocation_error(hPool->impl_->pool_priv);
}

void* umf_memory_pool_t::do_allocate(std::size_t bytes, std::size_t alignment) {
    return umfPoolAlignedMalloc(this, bytes, alignment);
}

void umf_memory_pool_t::do_deallocate(void* p, std::size_t, std::size_t)  {
    umfPoolFree(this, p);
}

bool umf_memory_pool_t::do_is_equal(const std::pmr::memory_resource& other ) const noexcept {
    auto mr = dynamic_cast<const umf_memory_pool_t*>(&other);
    return mr != nullptr && mr->impl_->pool_priv == impl_->pool_priv;
}

enum umf_result_t umfPoolCreate(const struct umf_memory_pool_ops_t *ops,
                                umf_memory_provider_handle_t *providers,
                                size_t numProviders, void *params,
                                umf_memory_pool_handle_t *hPool) {
    try {
        *hPool = new umf_memory_pool_t(ops, providers, numProviders, params);
        return UMF_RESULT_SUCCESS;
    } catch(umf_result_t ret) {
        return ret;
    }
}
