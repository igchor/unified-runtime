#include <umf/base.h>
#include <umf/memory_provider.h>

namespace umf
{
namespace polymorphic
{

struct memory_provider {
    virtual enum umf_result_t alloc(size_t, size_t, void **) = 0;
    virtual enum umf_result_t free(void *ptr, size_t size) = 0;
    virtual enum umf_result_t get_recommended_page_size(size_t size,
                                                size_t *pageSize)= 0;
    virtual enum umf_result_t get_min_page_size(void *ptr, size_t *pageSize) = 0;
    virtual enum umf_result_t purge_lazy(void *ptr, size_t size) = 0;
    virtual enum umf_result_t purge_force(void *ptr, size_t size) = 0;
    virtual const char *get_name() = 0;
};

// wraps c handle in c++ type
struct memory_provider_c : public memory_provider {
    memory_provider_c(umf_memory_provider_handle_t hProvider): hProvider(hProvider) {}
    enum umf_result_t alloc(size_t size, size_t align, void **ptr) {return umfMemoryProviderAlloc(hProvider, size, align, ptr);}
    enum umf_result_t free(void *ptr, size_t size) {return umfMemoryProviderFree(hProvider, ptr, size);}
    enum umf_result_t get_recommended_page_size(size_t size,
                                                size_t *pageSize) {return umfMemoryProviderGetRecommendedPageSize(hProvider, size, pageSize);}
    enum umf_result_t get_min_page_size(void *ptr, size_t *pageSize) {return umfMemoryProviderGetMinPageSize(hProvider, ptr, pageSize);}
    enum umf_result_t purge_lazy(void *ptr, size_t size) {return umfMemoryProviderPurgeLazy(hProvider, ptr, size);}
    enum umf_result_t purge_force(void *ptr, size_t size) {return umfMemoryProviderPurgeForce(hProvider, ptr, size);}
    const char *get_name() {return umfMemoryProviderGetName(hProvider);}
private:
    umf_memory_provider_handle_t hProvider;
};

}

// creates c handle from a c++ type, fills ops struct, call provider_create, etc.
template <typename Provider, typename... Args> // could be constrained by concept
umf_memory_provider_handle_t memoryProviderHandleCreate(Args&&...) {
    // implementation as in umf_helpers.hpp or similar
    return nullptr;
}

}
