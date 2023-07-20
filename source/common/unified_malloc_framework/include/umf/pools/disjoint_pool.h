#include <umf/base.h>
#include <umf/memory_provider.h>
#include <umf/memory_pool.h>

struct UmfDisjointPoolConfig {
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
};

umf_memory_pool_handle_t umfCreateDisjointPool(umf_memory_provider_handle_t upstream, umf_memory_provider_handle_t metadata, struct UmfDisjointPoolConfig *config);
