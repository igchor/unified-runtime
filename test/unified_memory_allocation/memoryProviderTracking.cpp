// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: MIT

#include "helpers.h"
#include "provider.h"

#include "uma_helpers.hpp"

#include <map>
#include <string>

TEST_F(umaTest, memoryProviderTrace) {
    void *pool = reinterpret_cast<void *>(123);
    static constexpr size_t size = 1024;

    uma_memory_tracker_handle_t tracker;
    auto ret = umaMemoryTrackerCreate(&tracker);
    ASSERT_EQ(ret, UMA_RESULT_SUCCESS);

    uma_memory_provider_handle_t handle;
    ret = umaTrackingMemoryProviderCreate(nullProviderCreate(), tracker, pool, &handle);
    ASSERT_EQ(ret, UMA_RESULT_SUCCESS);

    void *ptr;
    ret = umaMemoryProviderAlloc(handle, size, 0, &ptr);
    ASSERT_EQ(ret, UMA_RESULT_SUCCESS);

    auto foundPool = umaMemoryTrackerGetPool(tracker, ptr);
    ASSERT_EQ(foundPool, pool);

    foundPool = umaMemoryTrackerGetPool(tracker, reinterpret_cast<void *>(reinterpret_cast<char *>(ptr) + size - 1));
    ASSERT_EQ(foundPool, pool);

    auto nonExistentPool = umaMemoryTrackerGetPool(tracker, reinterpret_cast<void *>(reinterpret_cast<char *>(ptr) + size));
    ASSERT_EQ(nonExistentPool, nullptr);
}
