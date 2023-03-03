/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef UMA_HELPERS_H
#define UMA_HELPERS_H 1

#include <uma/memory_pool.h>
#include <uma/memory_pool_ops.h>
#include <uma/memory_provider.h>
#include <uma/memory_provider_ops.h>

#include <stdexcept>
#include <tuple>
#include <utility>

namespace uma {

// gives handle type given pointer to umaCreate* function
template <typename OpsT, typename HandleT>
auto handle_type(enum uma_result_t (*)(OpsT *, void *, HandleT *)) -> HandleT;

// gives return type given pointer to member function
template <typename T, typename ResultT, typename... Args>
auto function_return_type(ResultT (T::*)(Args...)) -> ResultT;

template <typename Ctor, typename Dtor, typename... Args>
auto makeUmaHandle(Ctor &&ctor, Dtor &&dtor, Args &&...args) {
    using handle_t = decltype(handle_type(ctor));
    using object_t = decltype(*std::declval<handle_t>());
    using unique_handle_t = std::unique_ptr<std::remove_reference_t<object_t>, std::remove_reference_t<decltype(dtor)>>;

    handle_t handle;
    auto ret = (*ctor)(args..., &handle);
    if (ret != UMA_RESULT_SUCCESS) {
        return std::pair<uma_result_t, unique_handle_t>(ret, unique_handle_t(nullptr, nullptr));
    }
    return std::pair<uma_result_t, unique_handle_t>(ret, unique_handle_t(handle, dtor));
}

// similar to std::make_from_tuple but uses operator new
template <typename T, typename Tuple, size_t... I>
auto createFromTuple(Tuple &&t, std::index_sequence<I...>) {
    return new T(std::get<I>(std::forward<Tuple>(t))...);
}
template <typename T, typename Tuple>
auto createFromTuple(Tuple &&t) {
    return createFromTuple<T>(
        std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size<Tuple>::value>{});
}

// Assigns lambda to 'OpsMember' field of ops that internally calls 'ObjT::ObjMember' method
template <typename ObjT, auto OpsMember, auto ObjMember, typename OpsT>
void umaOpsGenerateFunction(OpsT &ops) {
    ops.*OpsMember = [](void *obj, auto... args) {
        if constexpr (std::is_same_v<decltype(function_return_type(ObjMember)), void>) {
            try {
                (reinterpret_cast<ObjT *>(obj)->*ObjMember)(args...);
            } catch (...) {
                // TODO: log
            }
        } else if constexpr (std::is_same_v<decltype(function_return_type(ObjMember)), enum uma_result_t>) {
            try {
                return (reinterpret_cast<ObjT *>(obj)->*ObjMember)(args...);
            } catch (...) {
                return UMA_RESULT_ERROR_UNKNOWN;
            }
        } else {
            try {
                return (reinterpret_cast<ObjT *>(obj)->*ObjMember)(args...);
            } catch (...) {
                return decltype(function_return_type(ObjMember)){};
            }
        }
    };
}

#define UMA_OPS_GENERATE_FUNCTION(ObjT, member, ops) umaOpsGenerateFunction<ObjT, &decltype(ops)::member, &ObjT::member>(ops)

template <typename ObjT, typename OpsT, typename CtorT, typename DtorT, typename... Args>
auto createUmaObject(OpsT &ops, CtorT &&ctor, DtorT &&dtor, Args &&...args) {
    auto argsTuple = std::make_tuple(std::forward<Args>(args)..., (void *)nullptr);

    ops.version = UMA_VERSION_CURRENT;
    ops.initialize = [](void *params, void **obj) {
        try {
            auto *tuple = reinterpret_cast<decltype(argsTuple) *>(params);
            std::get<std::tuple_size_v<decltype(argsTuple)> - 1>(*tuple) = obj;
            *obj = createFromTuple<ObjT>(std::move(*tuple));
            return UMA_RESULT_SUCCESS;
        } catch (...) {
            return UMA_RESULT_ERROR_UNKNOWN;
        }
    };
    ops.finalize = [](void *obj) {
        delete reinterpret_cast<ObjT *>(obj);
    };
    return makeUmaHandle(ctor, dtor, &ops, reinterpret_cast<void *>(&argsTuple));
}

/// @brief creates UMA memory provider based on given MemProvider type.
/// MemProvider should implement all functions defined by
/// uma_memory_provider_ops_t, except for intialize and finalize (those are
/// replaced by ctor and dtor). All arguments passed to this function are
/// forwarded to MemProvider ctor.
template <typename MemProvider, typename... Args>
auto memoryProviderMakeUnique(Args &&...args) {
    uma_memory_provider_ops_t ops;

    UMA_OPS_GENERATE_FUNCTION(MemProvider, alloc, ops);
    UMA_OPS_GENERATE_FUNCTION(MemProvider, free, ops);
    UMA_OPS_GENERATE_FUNCTION(MemProvider, get_last_result, ops);

    return createUmaObject<MemProvider>(ops, &umaMemoryProviderCreate, &umaMemoryProviderDestroy, args...);
}

/// @brief creates UMA memory pool based on given MemPool type.
/// MemPool should implement all functions defined by
/// uma_memory_pool_ops_t, except for intialize and finalize (those are
/// replaced by ctor and dtor). All arguments passed to this function are
/// forwarded to MemProvider ctor.
template <typename MemPool, typename... Args>
auto poolMakeUnique(Args &&...args) {
    uma_memory_pool_ops_t ops;

    UMA_OPS_GENERATE_FUNCTION(MemPool, malloc, ops);
    UMA_OPS_GENERATE_FUNCTION(MemPool, aligned_malloc, ops);
    UMA_OPS_GENERATE_FUNCTION(MemPool, realloc, ops);
    UMA_OPS_GENERATE_FUNCTION(MemPool, malloc_usable_size, ops);
    UMA_OPS_GENERATE_FUNCTION(MemPool, free, ops);
    UMA_OPS_GENERATE_FUNCTION(MemPool, get_last_result, ops);

    return createUmaObject<MemPool>(ops, &umaPoolCreate, &umaPoolDestroy, args...);
}
} // namespace uma

#endif /* UMA_HELPERS_H */
