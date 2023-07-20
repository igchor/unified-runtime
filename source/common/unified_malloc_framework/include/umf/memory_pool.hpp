#include <umf/base.h>
#include <umf/memory_pool.h>
#include <umf/memory_provider.hpp>

namespace umf
{
namespace polymorphic
{

struct memory_pool {
    virtual void *malloc(size_t size) = 0;
    virtual void *calloc(size_t, size_t) = 0;
    virtual void *realloc(void *, size_t) = 0;
    virtual void *aligned_malloc(size_t size, size_t alignment) = 0;
    virtual size_t malloc_usable_size(void *) = 0;
    virtual void free(void *ptr) = 0;
};

// wraps c handle in c++ type
struct memory_pool_c : public memory_pool {
    memory_pool_c(umf_memory_pool_handle_t hPool): hPool(hPool) {}
    void *malloc(size_t size) {return umfPoolMalloc(hPool, size);} // TODO: check last_allocation_error and throw?
    void *calloc(size_t num, size_t size) {return umfPoolCalloc(hPool, num, size);} // TODO: check last_allocation_error and throw?
    void *realloc(void *ptr, size_t size) {return umfPoolRealloc(hPool, ptr, size);} // TODO: check last_allocation_error and throw?
    void *aligned_malloc(size_t size, size_t alignment) {return umfPoolAlignedMalloc(hPool, size, alignment);} // TODO: check last_allocation_error and throw?
    size_t malloc_usable_size(void *ptr) {return umfPoolMallocUsableSize(hPool, ptr);}
    void free(void *ptr) {return umfPoolFree(hPool, ptr);}
private:
    umf_memory_pool_handle_t hPool;
};

}

// creates c handle from a c++ type, fills ops struct, call pool_create, etc.
template <typename Pool, typename... Args> // could be constrained by concept
umf_memory_pool_handle_t memoryPoolHandleCreate(Args&&...) {
    // implementation as in umf_helpers.hpp or similar
    return nullptr;
}

}
