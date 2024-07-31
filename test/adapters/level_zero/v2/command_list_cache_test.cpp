// Copyright (C) 2024 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "command_list_cache.hpp"
#include "common.hpp"
#include "v2/context.hpp"

#include "context.hpp"
#include "device.hpp"

#include "uur/fixtures.h"
#include "uur/raii.h"

#include <gtest/gtest.h>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include "v2/latency_tracker.hpp"

struct CommandListCacheTest : public uur::urContextTest {};

UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(CommandListCacheTest);

TEST_P(CommandListCacheTest, CanStoreAndRetriveImmediateAndRegularCmdLists) {
    std::vector<int64_t> ns;
    for (int i = 0; i < 10000; i++) {
        std::chrono::time_point<std::chrono::steady_clock> begin = std::chrono::steady_clock::now();
        
        v2::latency_histogram histogram("name");

        auto tp = std::chrono::steady_clock::now();
        auto diffNanos =
          std::chrono::duration_cast<std::chrono::nanoseconds>(tp - begin)
              .count();
        ns.push_back(diffNanos);
    }

    std::cout << "mean: " << std::accumulate(ns.begin(), ns.end(), 0) / ns.size() << std::endl;
    std::cout << "stddev: " << std::sqrt(std::accumulate(ns.begin(), ns.end(), 0, [mean = std::accumulate(ns.begin(), ns.end(), 0) / ns.size()](int64_t a, int64_t b) { return a + (b - mean) * (b - mean); }) / ns.size()) << std::endl;

    v2::latency_histogram histogram("name");
    ns.clear();
    for (int i = 0; i < 10000; i++) {
        std::chrono::time_point<std::chrono::steady_clock> begin = std::chrono::steady_clock::now();

        histogram.trackValue(i);

                auto tp = std::chrono::steady_clock::now();
        auto diffNanos =
          std::chrono::duration_cast<std::chrono::nanoseconds>(tp - begin)
              .count();
        ns.push_back(diffNanos);
    }

    
    std::cout << "mean insert: " << std::accumulate(ns.begin(), ns.end(), 0) / ns.size() << std::endl;
    std::cout << "stddev insert: " << std::sqrt(std::accumulate(ns.begin(), ns.end(), 0, [mean = std::accumulate(ns.begin(), ns.end(), 0) / ns.size()](int64_t a, int64_t b) { return a + (b - mean) * (b - mean); }) / ns.size()) << std::endl;
}
