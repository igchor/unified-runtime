// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: MIT
// This file contains tests for UMA pool API

#include "pool.h"
#include "pool.hpp"
#include "provider.h"
#include "provider.hpp"

#include "memoryPool.hpp"

#include <string>
#include <unordered_map>

using uma_test::test;

TEST_F(test, memoryPoolTrace) {
    static std::unordered_map<std::string, size_t> calls;
    auto trace = [](const char *name) { calls[name]++; };

    auto nullPool = uma_test::wrapPoolUnique(nullPoolCreate());
    auto tracingPool = uma_test::wrapPoolUnique(tracePoolCreate(nullPool.get(), trace));

    size_t call_count = 0;

    umaPoolMalloc(tracingPool.get(), 0);
    ASSERT_EQ(calls["malloc"], 1);
    ASSERT_EQ(calls.size(), ++call_count);

    umaPoolFree(tracingPool.get(), nullptr);
    ASSERT_EQ(calls["free"], 1);
    ASSERT_EQ(calls.size(), ++call_count);

    umaPoolCalloc(tracingPool.get(), 0, 0);
    ASSERT_EQ(calls["calloc"], 1);
    ASSERT_EQ(calls.size(), ++call_count);

    umaPoolRealloc(tracingPool.get(), nullptr, 0);
    ASSERT_EQ(calls["realloc"], 1);
    ASSERT_EQ(calls.size(), ++call_count);

    umaPoolAlignedMalloc(tracingPool.get(), 0, 0);
    ASSERT_EQ(calls["aligned_malloc"], 1);
    ASSERT_EQ(calls.size(), ++call_count);

    umaPoolMallocUsableSize(tracingPool.get(), nullptr);
    ASSERT_EQ(calls["malloc_usable_size"], 1);
    ASSERT_EQ(calls.size(), ++call_count);

    enum uma_result_t ret = umaPoolGetLastResult(tracingPool.get(), nullptr);
    ASSERT_EQ(ret, UMA_RESULT_SUCCESS);
    ASSERT_EQ(calls["get_last_result"], 1);
    ASSERT_EQ(calls.size(), ++call_count);
}

TEST_F(test, memoryPoolWithCustomProviders) {
    struct uma_memory_provider_desc_t providers[] = {
        {nullProviderCreate(), UMA_MEMORY_PROVIDER_TYPE_DATA},
        {nullProviderCreate(), UMA_MEMORY_PROVIDER_TYPE_METADATA},
    };

    struct pool : public uma_test::pool_base {
        uma_result_t initialize(struct uma_memory_provider_desc_t *providers,
                                size_t numProviders) noexcept {
            EXPECT_NE(providers, nullptr);
            EXPECT_EQ(numProviders, 2);

            EXPECT_EQ(providers[0].providerType, UMA_MEMORY_PROVIDER_TYPE_DATA);
            EXPECT_EQ(providers[1].providerType, UMA_MEMORY_PROVIDER_TYPE_METADATA);
            return UMA_RESULT_SUCCESS;
        }
    };

    auto ret = uma::poolMakeUnique<pool>(providers, 2);
    ASSERT_EQ(ret.first, UMA_RESULT_SUCCESS);
    ASSERT_NE(ret.second, nullptr);

    for (auto &provider : providers) {
        umaMemoryProviderDestroy(provider.hProvider);
    }
}

template <typename Pool>
static auto makePool(std::function<uma::provider_unique_handle_t()> makeProvider) {
    auto provider = makeProvider();
    uma_memory_provider_desc_t desc{provider.get(), UMA_MEMORY_PROVIDER_TYPE_DATA};
    auto pool = uma::poolMakeUnique<Pool>(&desc, 1).second;
    auto dtor = [provider = provider.release()](uma_memory_pool_handle_t hPool) {
        umaPoolDestroy(hPool);
        umaMemoryProviderDestroy(provider);
    };
    return uma::pool_unique_handle_t(pool.release(), std::move(dtor));
}

INSTANTIATE_TEST_SUITE_P(mallocPoolTest, umaPoolTest,
                         ::testing::Values(
                             [] {
                                 return makePool<uma_test::malloc_pool>([] { return uma_test::wrapProviderUnique(nullProviderCreate()); });
                             }));

INSTANTIATE_TEST_SUITE_P(mallocProviderPoolTest, umaPoolTest,
                         ::testing::Values(
                             [] {
                                 return makePool<uma_test::proxy_pool>([] { return uma::memoryProviderMakeUnique<uma_test::provider_malloc>().second; });
                             }));

INSTANTIATE_TEST_SUITE_P(mallocMultiPoolTest, umaMultiPoolTest,
                         ::testing::Values(
                             [] {
                                 return makePool<uma_test::proxy_pool>([] { return uma::memoryProviderMakeUnique<uma_test::provider_malloc>().second; });
                             }));

//////////////////////////// Negative test cases ////////////////////////////////

TEST_F(test, memoryPoolInvalidNumProviders) {
    auto ret = uma::poolMakeUnique<uma_test::pool_base>(nullptr, 1);
    ASSERT_EQ(ret.first, UMA_RESULT_ERROR_INVALID_ARGUMENT);
}

TEST_F(test, memoryPoolInvalidProviders) {
    auto nullProvider = uma_test::wrapProviderUnique(nullProviderCreate());
    struct uma_memory_provider_desc_t providers[] = {
        {nullProvider.get(), UMA_MEMORY_PROVIDER_TYPE_DATA}};

    auto ret = uma::poolMakeUnique<uma_test::pool_base>(providers, 0);
    ASSERT_EQ(ret.first, UMA_RESULT_ERROR_INVALID_ARGUMENT);
}

struct poolInitializeTest : uma_test::test, ::testing::WithParamInterface<uma_result_t> {};

INSTANTIATE_TEST_SUITE_P(poolInitializeTest,
                         poolInitializeTest,
                         ::testing::Values(
                             UMA_RESULT_ERROR_OUT_OF_HOST_MEMORY,
                             UMA_RESULT_ERROR_POOL_SPECIFIC,
                             UMA_RESULT_ERROR_MEMORY_PROVIDER_SPECIFIC,
                             UMA_RESULT_ERROR_INVALID_ARGUMENT,
                             UMA_RESULT_ERROR_UNKNOWN));

TEST_P(poolInitializeTest, errorPropagation) {
    auto nullProvider = uma_test::wrapProviderUnique(nullProviderCreate());
    struct uma_memory_provider_desc_t providers[] = {
        {nullProvider.get(), UMA_MEMORY_PROVIDER_TYPE_DATA}};

    struct pool : public uma_test::pool_base {
        uma_result_t initialize(struct uma_memory_provider_desc_t *providers,
                                size_t numProviders, uma_result_t errorToReturn) noexcept {
            return errorToReturn;
        }
    };
    auto ret = uma::poolMakeUnique<pool>(providers, 1, this->GetParam());
    ASSERT_EQ(ret.first, this->GetParam());
    ASSERT_EQ(ret.second, nullptr);
}
