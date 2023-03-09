// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: MIT

#include "pool.hpp"

#include <cstring>
#include <functional>
#include <random>

#ifndef UMA_TEST_MEMORY_POOL_OPS_HPP
#define UMA_TEST_MEMORY_POOL_OPS_HPP

struct umaPoolTest : uma_test::test, ::testing::WithParamInterface<std::function<std::pair<uma_result_t, uma::pool_unique_handle_t>()>> {
    umaPoolTest() : pool(nullptr, nullptr) {}
    void SetUp() {
        test::SetUp();
        this->pool = makePool();
    }

    void TearDown() override {
        test::TearDown();
    }

    uma::pool_unique_handle_t makePool() {
        auto [res, pool] = this->GetParam()();
        EXPECT_EQ(res, UMA_RESULT_SUCCESS);
        EXPECT_NE(pool, nullptr);
        return std::move(pool);
    }

    uma::pool_unique_handle_t pool;
};

struct umaMultiPoolTest : umaPoolTest {
    static constexpr auto numPools = 16;

    void SetUp() {
        umaPoolTest::SetUp();

        pools.emplace_back(std::move(pool));
        for (size_t i = 1; i < numPools; i++) {
            pools.emplace_back(makePool());
        }
    }

    void TearDown() override {
        umaPoolTest::TearDown();
    }

    std::vector<uma::pool_unique_handle_t> pools;
};

TEST_P(umaPoolTest, allocFree) {
    static constexpr size_t allocSize = 64;
    auto *ptr = umaPoolMalloc(pool.get(), allocSize);
    ASSERT_NE(ptr, nullptr);
    std::memset(ptr, 0, allocSize);
    umaPoolFree(pool.get(), ptr);
}

TEST_P(umaPoolTest, pow2AlignedAlloc) {
#ifdef _WIN32
    // TODO: implement support for windows
    GTEST_SKIP();
#endif

    static constexpr size_t allocSize = 64;
    static constexpr size_t maxAlignment = (1u << 22);

    for (size_t alignment = 1; alignment <= maxAlignment; alignment <<= 1) {
        std::cout << alignment << std::endl;
        auto *ptr = umaPoolAlignedMalloc(pool.get(), allocSize, alignment);
        ASSERT_NE(ptr, nullptr);
        ASSERT_TRUE(reinterpret_cast<uintptr_t>(ptr) % alignment == 0);
        std::memset(ptr, 0, allocSize);
        umaPoolFree(pool.get(), ptr);
    }
}

// TODO: add similar tests for realloc/aligned_alloc, etc.
// TODO: add multithreaded tests
TEST_P(umaMultiPoolTest, memoryTracking) {
    static constexpr int allocSizes[] = {1, 2, 4, 8, 16, 20, 32, 40, 64, 128, 1024, 4096};
    static constexpr auto nAllocs = 256;

    std::mt19937_64 g(0); // TODO: test with random seeds?
    std::uniform_int_distribution allocSizesDist(0, static_cast<int>(std::size(allocSizes) - 1));
    std::uniform_int_distribution poolsDist(0, static_cast<int>(pools.size() - 1));

    std::vector<std::tuple<void *, size_t, uma_memory_pool_handle_t>> ptrs;
    for (size_t i = 0; i < nAllocs; i++) {
        auto &pool = pools[poolsDist(g)];
        auto size = allocSizes[allocSizesDist(g)];

        auto *ptr = umaPoolMalloc(pool.get(), size);
        ASSERT_NE(ptr, nullptr);

        ptrs.emplace_back(ptr, size, pool.get());
    }

    for (auto [ptr, size, expectedPool] : ptrs) {
        auto pool = umaPoolByPtr(ptr);
        ASSERT_EQ(pool, expectedPool);

        pool = umaPoolByPtr(reinterpret_cast<void *>(reinterpret_cast<intptr_t>(ptr) + size - 1));
        ASSERT_EQ(pool, expectedPool);
    }

    for (auto [ptr, _1, _2] : ptrs) {
        umaFree(ptr);
    }
}

#endif /* UMA_TEST_MEMORY_POOL_OPS_HPP */
