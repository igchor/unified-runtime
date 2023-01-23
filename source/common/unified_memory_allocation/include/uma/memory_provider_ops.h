/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UMA_MEMORY_PROVIDER_OPS_H
#define UMA_MEMORY_PROVIDER_OPS_H 1

#include <uma/base.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO: create macro for intializing this structure (including version fields)
struct uma_memory_provider_ops_t {
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

  /// Refer to memory_provider.h for description of those functions
  enum uma_result_t (*alloc)(void *provider, size_t size, size_t alignment,
                             void **ptr);
  enum uma_result_t (*free)(void *provider, void *ptr, size_t size);
};

#ifdef __cplusplus
}
#endif

#endif /* #ifndef UMA_MEMORY_PROVIDER_OPS_H */
