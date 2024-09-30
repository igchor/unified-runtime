// Copyright (C) 2024 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "ur_print.hpp"
#include "uur/fixtures.h"
#include "uur/raii.h"
#include "uur/utils.h"

#include <map>
#include <string>

using urMemoryResidencyTest = uur::urMultiDeviceContextTestTemplate<1>;

TEST_F(urMemoryResidencyTest, allocatingDeviceMemoryWillResultInOOM) {
    static constexpr size_t allocSize = 1024 * 1024 * 1024;
    static constexpr size_t maxNumAllocs = 1024 * 1024;

    if (!uur::isPVC(uur::DevicesEnvironment::instance->devices[0])) {
        GTEST_SKIP() << "Test requires a PVC device";
    }

    size_t globalMemFree = 0;
    ASSERT_SUCCESS(
        urDeviceGetInfo(uur::DevicesEnvironment::instance->devices[0],
                        UR_DEVICE_INFO_GLOBAL_MEM_FREE, sizeof(size_t),
                        &globalMemFree, nullptr));

    if (globalMemFree < allocSize) {
        GTEST_SKIP() << "Not enough device memory available";
    }

    std::vector<void *> allocations;
    for (size_t i = 0; i < maxNumAllocs; i++) {
        void *ptr = nullptr;
        auto status = urUSMDeviceAlloc(
            context, uur::DevicesEnvironment::instance->devices[0], nullptr,
            nullptr, allocSize, &ptr);

        ASSERT_SUCCESS(
            urDeviceGetInfo(uur::DevicesEnvironment::instance->devices[0],
                            UR_DEVICE_INFO_GLOBAL_MEM_FREE, sizeof(size_t),
                            &globalMemFree, nullptr));

        if (status == UR_RESULT_ERROR_OUT_OF_DEVICE_MEMORY) {
            ASSERT_GT(i, 0); // first allocation shouldn't fail
            ASSERT_LE(globalMemFree, allocSize);

            for (auto &alloc : allocations) {
                urUSMFree(context, alloc);
            }

            std::cout << "OOM after " << i << " allocations" << std::endl;

            // We expect to run out of memory before reaching the maximum number of allocations
            return;
        }
        ASSERT_EQ(UR_RESULT_SUCCESS, status);
        allocations.push_back(ptr);
    }

    FAIL() << "Expected to run out of device memory";
}
