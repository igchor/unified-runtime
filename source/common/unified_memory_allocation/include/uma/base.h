/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UMA_BASE_H
#define UMA_BASE_H 1

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UMA_VERSION_MAJOR 0
#define UMA_VERSION_MINOR 9

/// \brief Operation statuses
enum uma_result_t {
  /**
   * @brief Success.
   */
  UMA_RESULT_SUCCESS = 0,

  /**
   * @brief Error: Operation failed.
   */
  UMA_RESULT_OPERATION_FAILED = -1,

  /**
   * @brief Error: pointer argument may not be nullptr
   */
  UMA_RESULT_ERROR_INVALID_NULL_POINTER = -2,

  /**
   * @brief Error: handle argument is not valid
   */
  UMA_RESULT_ERROR_INVALID_NULL_HANDLE = -3,

  /**
   * @brief Error: Unspecified run-time error.
   */
  UMA_RESULT_RUNTIME_ERROR = -255
};

#ifdef __cplusplus
}
#endif

#endif /* UMA_BASE_H */
