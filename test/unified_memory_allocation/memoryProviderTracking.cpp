// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: MIT

#include "helpers.h"
#include "provider.h"

#include "uma_helpers.hpp"
#include "uma_tracker.hpp"

#include <map>
#include <string>

TEST_F(umaTest, memoryProviderTrace) {
    void *pool = reinterpret_cast<void *>(123);
    static constexpr size_t size = 1024;

    auto tracker = uma::trackerMakeUnique<uma::memory_tracker>();
    ASSERT_EQ(tracker.first, UMA_RESULT_SUCCESS);
    ASSERT_NE(tracker.second, nullptr);

    uma_memory_provider_handle_t handle;
    auto ret = umaTrackingMemoryProviderCreate(nullProviderCreate(), tracker.second.get(), pool, &handle);
    ASSERT_EQ(ret, UMA_RESULT_SUCCESS);

    void *ptr;
    ret = umaMemoryProviderAlloc(handle, size, 0, &ptr);
    ASSERT_EQ(ret, UMA_RESULT_SUCCESS);

    auto foundPool = umaMemoryTrackerFind(tracker.second.get(), ptr);
    ASSERT_EQ(foundPool, pool);

    foundPool = umaMemoryTrackerFind(tracker.second.get(), reinterpret_cast<void *>(reinterpret_cast<char *>(ptr) + size - 1));
    ASSERT_EQ(foundPool, pool);

    auto nonExistentPool = umaMemoryTrackerFind(tracker.second.get(), reinterpret_cast<void *>(reinterpret_cast<char *>(ptr) + size));
    ASSERT_EQ(nonExistentPool, nullptr);
}
