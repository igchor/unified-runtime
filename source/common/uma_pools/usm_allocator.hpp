//===---------- usm_allocator.hpp - Allocator for USM memory --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef USM_ALLOCATOR
#define USM_ALLOCATOR

#include <atomic>
#include <memory>

#include "uma_helpers.hpp"

class USMLimits {
  public:
    USMLimits() : TotalSize(0) {}

    // Maximum memory left unfreed
    size_t MaxSize = 16 * 1024 * 1024;

    // Total size of pooled memory
    std::atomic<size_t> TotalSize;
};

// Configuration for specific USM allocator instance
class USMAllocatorParameters {
  public:
    USMAllocatorParameters();

    const char *memoryTypeName = "";

    // Minimum allocation size that will be requested from the system.
    // By default this is the minimum allocation size of each memory type.
    size_t SlabMinSize = 0;

    // Allocations up to this limit will be subject to chunking/pooling
    size_t MaxPoolableSize = 0;

    // When pooling, each bucket will hold a max of 4 unfreed slabs
    size_t Capacity = 0;

    // Holds the minimum bucket size valid for allocation of a memory type.
    size_t MinBucketSize = 0;

    // Holds size of the pool managed by the allocator.
    size_t CurPoolSize = 0;

    // Whether to print pool usage statistics
    int PoolTrace = 0;

    std::shared_ptr<USMLimits> limits;
};

// TODO: get rid of this and only leave usm_pool once L0 adapter is implemented
class USMAllocContext {
  public:
    // Keep it public since it needs to be accessed by the lower layer(Buckets)
    class USMAllocImpl;

    USMAllocContext(uma::ur_memory_provider_handle_unique hProvider,
                    USMAllocatorParameters params);
    ~USMAllocContext();

    void *allocate(size_t size);
    void *allocate(size_t size, size_t alignment);
    void deallocate(void *ptr);

  private:
    std::unique_ptr<USMAllocImpl> pImpl;
};

#endif
