/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
 * See LICENSE.TXT
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 */

#ifndef UMA_MEMORY_POOL_H
#define UMA_MEMORY_POOL_H 1

#include <uma/base.h>
#include <uma/memory_provider.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct uma_memory_pool_t *uma_memory_pool_handle_t;

struct uma_memory_pool_ops_t;

///
/// \brief Creates new memory pool.
/// \param ops instance of uma_memory_pool_ops_t
/// \param data_provider memory provider that should be used for coarse-grain data allocation
/// \param metadata_provider [optional] memory provider that should be used for metadata allocations
/// \param params pointer to pool-specific parameters
/// \param hPool [out] handle to the newly created memory pool
/// \return UMA_RESULT_SUCCESS on success or appropriate error code on failure.
///
enum uma_result_t umaPoolCreate(struct uma_memory_pool_ops_t *ops,
                                uma_memory_provider_handle_t data_provider,
                                uma_memory_provider_handle_t metadata_provider,
                                void *params, uma_memory_pool_handle_t *hPool);

///
/// \brief Destroys memory pool.
/// \param hPool handle to the pool
///
void umaPoolDestroy(uma_memory_pool_handle_t hPool);

///
/// \brief Allocates size bytes of uninitialized storage of the specified hPool.
/// \param hPool specified memory hPool
/// \param size number of bytes to allocate
/// \return Pointer to the allocated memory.
///
void *umaPoolMalloc(uma_memory_pool_handle_t hPool, size_t size);

///
/// \brief Allocates size bytes of uninitialized storage of the specified hPool.
/// with specified alignment
/// \param hPool specified memory hPool
/// \param size number of bytes to allocate
/// \param alignment alignment of the allocation
/// \return Pointer to the allocated memory.
///
void *umaPoolAlignedMalloc(uma_memory_pool_handle_t hPool, size_t size,
                           size_t alignment);

///
/// \brief Allocates memory of the specified hPool for an array of num elements
///        of size bytes each and initializes all bytes in the allocated storage
///        to zero.
/// \param hPool specified memory hPool
/// \param num number of objects
/// \param size specified size of each element
/// \return Pointer to the allocated memory.
///
void *umaPoolCalloc(uma_memory_pool_handle_t hPool, size_t num, size_t size);

///
/// \brief Reallocates memory of the specified hPool.
/// \param hPool specified memory hPool
/// \param ptr pointer to the memory block to be reallocated
/// \param size new size for the memory block in bytes
/// \return Pointer to the allocated memory.
///
void *umaPoolRealloc(uma_memory_pool_handle_t hPool, void *ptr, size_t size);

///
/// \brief Obtains size of block of memory allocated from the pool.
/// \param hPool specified memory hPool
/// \param ptr pointer to the allocated memory
/// \return Number of bytes.
///
size_t umaPoolMallocUsableSize(uma_memory_pool_handle_t hPool, void *ptr);

///
/// \brief Frees the memory space of the specified hPool pointed by ptr.
/// \param hPool specified memory hPool
/// \param ptr pointer to the allocated memory
///
void umaPoolFree(uma_memory_pool_handle_t hPool, void *ptr);

///
/// \brief Frees the memory space pointed by ptr if it belongs to UMA pool, does nothing otherwise.
/// \param ptr pointer to the allocated memory
///
void umaFree(void *ptr);

///
/// \brief Retrieve string representation of the underlying pool specific
///        result reported by the last API that returned
///        UMA_RESULT_ERROR_POOL_SPECIFIC or NULL ptr (in case of allocation
///        functions). Allows for a pool independent way to
///        return a pool specific result.
///
/// \details
///     - The string returned via the ppMessage is a NULL terminated C style
///       string.
///     - The string returned via the ppMessage is thread local.
///     - The memory in the string returned via the ppMessage is owned by the
///       adapter.
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// \param hPool specified memory hPool
/// \param ppMessage [out] pointer to a string containing provider specific
///                  result in string representation.
/// \return UMA_RESULT_SUCCESS if the result being
///         reported is to be considered a warning. Any other result code
///         returned indicates that the adapter specific result is an error.
enum uma_result_t umaPoolGetLastResult(uma_memory_pool_handle_t hPool,
                                       const char **ppMessage);

///
/// \brief Retrieve memory pool associated with a given ptr.
/// \param ptr pointer to memory belonging to a memory pool
/// \return Handle to a memory pool that contains ptr or NULL if pointer does not belong to any UMA pool.
uma_memory_pool_handle_t umaPoolByPtr(const void *ptr);

///
/// \brief Retrieve metadata memory provider associated with a given pool.
/// \param hPool specified memory pool
/// \return handle to the metadata memory provider
uma_memory_provider_handle_t
umaPoolGetMetadataMemoryProvider(uma_memory_pool_handle_t hPool);

///
/// \brief Retrieve data memory provider associated with a given pool.
/// \param hPool specified memory pool
/// \return handle to the data memory provider
uma_memory_provider_handle_t
umaPoolGetDataMemoryProvider(uma_memory_pool_handle_t hPool);

#ifdef __cplusplus
}
#endif

#endif /* UMA_MEMORY_POOL_H */
