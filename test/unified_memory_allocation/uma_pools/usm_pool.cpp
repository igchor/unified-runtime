// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: MIT

#include "pool.h"
#include "pool.hpp"
#include "provider.h"
#include "provider.hpp"

#include "usm_allocator_config.hpp"
#include "usm_pool.hpp"

#include <cstring>

using uma_test::test;

TEST_F(test, usmPoolBasic) {
    auto provider = uma_test::wrapProviderUnique(mallocProviderCreate());
    auto usmParams = USMAllocatorParameters{};

    usmParams.SlabMinSize = 4096;
    usmParams.MaxPoolableSize = 4096;
    usmParams.Capacity = 4;
    usmParams.MinBucketSize = 64;

    uma_memory_provider_handle_t providers[] = {provider.get()};
    auto pool = uma::poolMakeUnique<UsmPool>(providers, 1, usmParams);

    static constexpr size_t allocSize = 64;
    auto *ptr = umaPoolMalloc(pool.get(), allocSize);

    std::memset(ptr, 0, allocSize);

    umaPoolFree(pool.get(), ptr);
}
