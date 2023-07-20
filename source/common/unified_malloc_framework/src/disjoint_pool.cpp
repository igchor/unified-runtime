#include "disjoint_pool.h"
#include "disjoint_pool.hpp"

#include "memory_pool.hpp"
#include "memory_provider.hpp"

extern "C" umf_memory_pool_handle_t 
umfCreateDisjointPool(umf_memory_provider_handle_t upstream, umf_memory_provider_handle_t metadata, struct UmfDisjointPoolConfig *config) {
    umf::DisjointPoolConfig cpp_config;
    // translate config to cpp_config
    cpp_config.Capacity = config->Capacity;
    // ...

    return umf::memoryPoolHandleCreate<umf::DisjointPool>(
        std::unique_ptr<umf::polymorphic::memory_provider>(new umf::polymorphic::memory_provider_c(upstream)),
        std::unique_ptr<umf::polymorphic::memory_provider>(new umf::polymorphic::memory_provider_c(metadata)),
        cpp_config);
}
