//===---------- disjoint_pool.hpp - Allocator for USM memory --------------===//
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef DISJOINT_POOL
#define DISJOINT_POOL

#include <atomic>
#include <memory>
#include <string>

#include <umf/base.h>
#include <umf/memory_provider.hpp>

namespace umf {

// Configuration for specific USM allocator instance
class DisjointPoolConfig {
  public:
    DisjointPoolConfig();

    std::string name = "";

    struct SharedLimits {
        SharedLimits() : TotalSize(0) {}

        // Maximum memory left unfreed
        size_t MaxSize = 16 * 1024 * 1024;

        // Total size of pooled memory
        std::atomic<size_t> TotalSize;
    };

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

    std::shared_ptr<SharedLimits> limits;
};

class DisjointPool {
  public:
    class AllocImpl;
    using Config = DisjointPoolConfig;

    // DisjointPool accepts providers as pointers to polymorphic types but other implementations could
    // just accept a template arguments that match 'memory_provider' concept (or pass some other checks pre-c++20)
    DisjointPool(std::unique_ptr<polymorphic::memory_provider> upstream, std::unique_ptr<polymorphic::memory_provider> metadata, DisjointPoolConfig parameters);
    void *malloc(size_t size);
    void *calloc(size_t, size_t);
    void *realloc(void *, size_t);
    void *aligned_malloc(size_t size, size_t alignment);
    size_t malloc_usable_size(void *);
    void free(void *ptr);

    DisjointPool();
    ~DisjointPool();

  private:
    std::unique_ptr<AllocImpl> impl;
};

} // namespace umf

#endif
