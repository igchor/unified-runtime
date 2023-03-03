/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UMA_OBJECT_H
#define UMA_OBJECT_H 1

#include <uma/base.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UMA_TYPE(NAME) struct uma_##NAME##_t
#define UMA_HANDLE(NAME) uma_##NAME##_handle_t
#define UMA_OPS(NAME) struct uma_##NAME##_ops_t

#define UMA_DECLARE_TYPE(NAME) \
    UMA_TYPE(NAME) {           \
        void *priv;            \
        UMA_OPS(NAME)          \
        ops;                   \
    }

#define UMA_CREATE_OBJECT(NAME, ops, params, ret)              \
    UMA_HANDLE(NAME)                                           \
    obj = malloc(sizeof(UMA_TYPE(NAME)));                      \
    if (!obj) {                                                \
        return UMA_RESULT_ERROR_OUT_OF_HOST_MEMORY;            \
    }                                                          \
    assert(ops->version == UMA_VERSION_CURRENT);               \
    obj->ops = *ops;                                           \
    enum uma_result_t r = ops->initialize(params, &obj->priv); \
    if (r != UMA_RESULT_SUCCESS) {                             \
        return r;                                              \
    }                                                          \
    *ret = obj;                                                \
    return UMA_RESULT_SUCCESS

#define UMA_DESTROY_OBJECT(obj)   \
    obj->ops.finalize(obj->priv); \
    free(obj)

#define UMA_FORWARD_CALL(obj, FUNC_NAME, ...) \
    obj->ops.FUNC_NAME(obj->priv, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* UMA_OBJECT_H */
