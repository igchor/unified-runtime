// Copyright (C) 2023 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// This file contains tests for UMA pool API

#include "pool.h"
#include "pool.hpp"
#include "provider.h"
#include "provider.hpp"

#include "memoryPool.hpp"
#include "uma/memory_provider.h"

#include <array>
#include <string>
#include <unordered_map>

using uma_test::test;

TEST_F(test, memoryPoolTrace) {
    static std::unordered_map<std::string, size_t> poolCalls;
    static std::unordered_map<std::string, size_t> providerCalls;
    auto tracePool = [](const char *name) { poolCalls[name]++; };
    auto traceProvider = [](const char *name) { providerCalls[name]++; };

    auto nullProvider = uma_test::wrapProviderUnique(nullProviderCreate());
    auto tracingProvider = uma_test::wrapProviderUnique(
        traceProviderCreate(nullProvider.get(), traceProvider));
    auto provider = tracingProvider.get();

    auto [ret, proxyPool] =
        uma::poolMakeUnique<uma_test::proxy_pool>(provider, nullptr);
    ASSERT_EQ(ret, UMA_RESULT_SUCCESS);

    auto tracingPool =
        uma_test::wrapPoolUnique(tracePoolCreate(proxyPool.get(), tracePool));

    size_t pool_call_count = 0;
    size_t provider_call_count = 0;

    umaPoolMalloc(tracingPool.get(), 0);
    ASSERT_EQ(poolCalls["malloc"], 1);
    ASSERT_EQ(poolCalls.size(), ++pool_call_count);

    ASSERT_EQ(providerCalls["alloc"], 1);
    ASSERT_EQ(providerCalls.size(), ++provider_call_count);

    umaPoolFree(tracingPool.get(), nullptr);
    ASSERT_EQ(poolCalls["free"], 1);
    ASSERT_EQ(poolCalls.size(), ++pool_call_count);

    ASSERT_EQ(providerCalls["free"], 1);
    ASSERT_EQ(providerCalls.size(), ++provider_call_count);

    umaPoolCalloc(tracingPool.get(), 0, 0);
    ASSERT_EQ(poolCalls["calloc"], 1);
    ASSERT_EQ(poolCalls.size(), ++pool_call_count);

    ASSERT_EQ(providerCalls["alloc"], 2);
    ASSERT_EQ(providerCalls.size(), provider_call_count);

    umaPoolRealloc(tracingPool.get(), nullptr, 0);
    ASSERT_EQ(poolCalls["realloc"], 1);
    ASSERT_EQ(poolCalls.size(), ++pool_call_count);

    ASSERT_EQ(providerCalls.size(), provider_call_count);

    umaPoolAlignedMalloc(tracingPool.get(), 0, 0);
    ASSERT_EQ(poolCalls["aligned_malloc"], 1);
    ASSERT_EQ(poolCalls.size(), ++pool_call_count);

    ASSERT_EQ(providerCalls["alloc"], 3);
    ASSERT_EQ(providerCalls.size(), provider_call_count);

    umaPoolMallocUsableSize(tracingPool.get(), nullptr);
    ASSERT_EQ(poolCalls["malloc_usable_size"], 1);
    ASSERT_EQ(poolCalls.size(), ++pool_call_count);

    ASSERT_EQ(providerCalls.size(), provider_call_count);

    ret = umaPoolGetLastResult(tracingPool.get(), nullptr);
    ASSERT_EQ(ret, UMA_RESULT_SUCCESS);

    ASSERT_EQ(poolCalls["get_last_result"], 1);
    ASSERT_EQ(poolCalls.size(), ++pool_call_count);

    ASSERT_EQ(providerCalls["get_last_result"], 1);
    ASSERT_EQ(providerCalls.size(), ++provider_call_count);

    auto p = umaPoolGetDataMemoryProvider(tracingPool.get());
    ASSERT_NE(p, nullptr);
    ASSERT_EQ(poolCalls["get_data_memory_provider"], 1);
    ASSERT_EQ(poolCalls.size(), ++pool_call_count);

    p = umaPoolGetMetadataMemoryProvider(tracingPool.get());
    ASSERT_EQ(poolCalls["get_metadata_memory_provider"], 1);
    ASSERT_EQ(poolCalls.size(), ++pool_call_count);
}

TEST_F(test, retrieveMemoryProviders) {
    struct memory_provider : uma_test::provider_base {
        const char *name;
        uma_result_t initialize(const char *name) {
            this->name = name;
            return UMA_RESULT_SUCCESS;
        }
        const char *get_name() noexcept { return name; }
    };

    auto [ret1, data_provider] =
        uma::memoryProviderMakeUnique<memory_provider>("data_provider");
    auto [ret2, metadata_provider] =
        uma::memoryProviderMakeUnique<memory_provider>("metadata_provider");

    ASSERT_EQ(ret1, UMA_RESULT_SUCCESS);
    ASSERT_EQ(ret2, UMA_RESULT_SUCCESS);

    auto [ret, pool] = uma::poolMakeUnique<uma_test::proxy_pool>(
        data_provider.get(), metadata_provider.get());

    ASSERT_EQ(ret, UMA_RESULT_SUCCESS);

    uma_memory_provider_handle_t ret_data_provider =
        umaPoolGetDataMemoryProvider(pool.get());
    uma_memory_provider_handle_t ret_metadata_provider =
        umaPoolGetMetadataMemoryProvider(pool.get());

    // comapre names of the providers: handles might point to a different object if providers are wrapped by memory tracking-provider
    ASSERT_EQ(std::string_view(umaMemoryProviderGetName(data_provider.get())),
              std::string_view(umaMemoryProviderGetName(ret_data_provider)));
    ASSERT_EQ(
        std::string_view(umaMemoryProviderGetName(metadata_provider.get())),
        std::string_view(umaMemoryProviderGetName(ret_metadata_provider)));
}

template <typename Pool>
static auto
makePool(std::function<uma::provider_unique_handle_t()> makeProvider) {
    auto providerUnique = makeProvider();
    uma_memory_provider_handle_t provider = providerUnique.get();
    auto pool = uma::poolMakeUnique<Pool>(provider, nullptr).second;
    auto dtor = [provider =
                     providerUnique.release()](uma_memory_pool_handle_t hPool) {
        umaPoolDestroy(hPool);
        umaMemoryProviderDestroy(provider);
    };
    return uma::pool_unique_handle_t(pool.release(), std::move(dtor));
}

INSTANTIATE_TEST_SUITE_P(mallocPoolTest, umaPoolTest, ::testing::Values([] {
                             return makePool<uma_test::malloc_pool>([] {
                                 return uma_test::wrapProviderUnique(
                                     nullProviderCreate());
                             });
                         }));

INSTANTIATE_TEST_SUITE_P(
    mallocProviderPoolTest, umaPoolTest, ::testing::Values([] {
        return makePool<uma_test::proxy_pool>([] {
            return uma::memoryProviderMakeUnique<uma_test::provider_malloc>()
                .second;
        });
    }));

INSTANTIATE_TEST_SUITE_P(
    mallocMultiPoolTest, umaMultiPoolTest, ::testing::Values([] {
        return makePool<uma_test::proxy_pool>([] {
            return uma::memoryProviderMakeUnique<uma_test::provider_malloc>()
                .second;
        });
    }));

////////////////// Negative test cases /////////////////

TEST_F(test, memoryPoolInvalidProvidersNullptr) {
    auto ret = uma::poolMakeUnique<uma_test::pool_base>(nullptr, nullptr);
    ASSERT_EQ(ret.first, UMA_RESULT_ERROR_INVALID_ARGUMENT);
}

struct poolInitializeTest : uma_test::test,
                            ::testing::WithParamInterface<uma_result_t> {};

INSTANTIATE_TEST_SUITE_P(
    poolInitializeTest, poolInitializeTest,
    ::testing::Values(UMA_RESULT_ERROR_OUT_OF_HOST_MEMORY,
                      UMA_RESULT_ERROR_POOL_SPECIFIC,
                      UMA_RESULT_ERROR_INVALID_ARGUMENT,
                      UMA_RESULT_ERROR_METADATA_MEMORY_PROVIDER_SPECIFIC,
                      UMA_RESULT_ERROR_DATA_MEMORY_PROVIDER_SPECIFIC,
                      UMA_RESULT_ERROR_UNKNOWN));

TEST_P(poolInitializeTest, errorPropagation) {
    auto nullProvider = uma_test::wrapProviderUnique(nullProviderCreate());
    struct pool : public uma_test::pool_base {
        uma_result_t initialize(uma_memory_provider_handle_t data_provider,
                                uma_memory_provider_handle_t metadata_provider,
                                uma_result_t errorToReturn) noexcept {
            return errorToReturn;
        }
    };
    auto ret = uma::poolMakeUnique<pool>(nullProvider.get(), nullptr,
                                         this->GetParam());
    ASSERT_EQ(ret.first, this->GetParam());
    ASSERT_EQ(ret.second, nullptr);
}
