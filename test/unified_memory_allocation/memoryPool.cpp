// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: MIT

#include "helpers.h"
#include "pool.h"
#include "provider.h"

#include "proxyPool.hpp"
#include "uma_helpers.hpp"
#include "uma_tracker.hpp"

#include <string>
#include <unordered_map>

TEST_F(umaTest, memoryPoolTrace) {
    static std::unordered_map<std::string, size_t> calls;
    auto trace = [](const char *name) { calls[name]++; };

    auto nullPool = uma::wrapPoolUnique(nullPoolCreate());
    auto tracingPool = uma::wrapPoolUnique(tracePoolCreate(nullPool.get(), trace));

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

TEST_F(umaTest, memoryPoolWithMemoryTracking) {
    static constexpr size_t size = 1024;

    auto trackerRet = uma::trackerMakeUnique<uma::memory_tracker>();
    ASSERT_EQ(trackerRet.first, UMA_RESULT_SUCCESS);
    ASSERT_NE(trackerRet.second, nullptr);
    auto &tracker = trackerRet.second;

    auto poolRet = uma::poolMakeUnique<uma_test::proxy_pool>(mallocProviderCreate(), tracker.get());
    ASSERT_EQ(poolRet.first, UMA_RESULT_SUCCESS);
    ASSERT_NE(poolRet.second, nullptr);
    auto &pool = poolRet.second;

    auto *ptr = umaPoolMalloc(pool.get(), size);
    ASSERT_NE(ptr, nullptr);

    auto foundPool = umaMemoryTrackerFind(tracker.get(), ptr);
    ASSERT_EQ(foundPool, pool.get());

    foundPool = umaMemoryTrackerFind(tracker.get(), reinterpret_cast<void *>(reinterpret_cast<char *>(ptr) + size - 1));
    ASSERT_EQ(foundPool, pool.get());

    auto nonExistentPool = umaMemoryTrackerFind(tracker.get(), reinterpret_cast<void *>(reinterpret_cast<char *>(ptr) + size));
    ASSERT_EQ(nonExistentPool, nullptr);
}
