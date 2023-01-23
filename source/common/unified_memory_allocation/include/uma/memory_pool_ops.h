/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UMA_MEMORY_POOL_OPS_H
#define UMA_MEMORY_POOL_OPS_H 1

#include <uma/base.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO: create macro for intializing this structure (including version fields)
struct uma_memory_pool_ops_t {
  uint64_t version_major;
  uint64_t version_minor;

  ///
  /// \brief intializes memory pool
  /// \param params pool-specific params
  /// \param pool returns pointer to the pool
  /// \return UMA_RESULT_SUCCESS on success or appropriate error code on failure
  enum uma_result_t (*initialize)(void *params, void **pool);

  ///
  /// \brief finalizes memory pool
  /// \param pool pool to finalize
  void (*finalize)(void *pool);

  /// Refer to memory_pool.h for description of those functions
  void *(*malloc)(void *pool, size_t size);
  void *(*calloc)(void *pool, size_t num, size_t size);
  void *(*realloc)(void *pool, void *ptr, size_t size);
  void *(*aligned_malloc)(void *pool, size_t size, size_t alignment);
  size_t (*malloc_usable_size)(void *pool, void *ptr);
  void (*free)(void *pool, void *);
};

#ifdef __cplusplus
}
#endif

#endif /* UMA_MEMORY_POOL_OPS_H */
