// Copyright (C) 2024 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "uur/fixtures.h"

#include <gtest/gtest.h>
#include <map>
#include <string>
#include <unordered_set>

struct EnqueueKernelLaunchTest : public uur::urQueueTest {};

UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(EnqueueKernelLaunchTest);

TEST_P(EnqueueKernelLaunchTest, Success) {
    void *ptr;
    ASSERT_SUCCESS(
        urUSMSharedAlloc(context, device, nullptr, nullptr, 1024,
                            &ptr));

    ASSERT_NE(ptr, nullptr);

    uint8_t pattern = 0x4;
    ASSERT_SUCCESS(urEnqueueUSMFill(queue, ptr, sizeof(pattern), &pattern, 1024, 0, nullptr, nullptr));

    ASSERT_SUCCESS(urQueueFinish(queue));

    ASSERT_EQ(0, memcmp(ptr, &pattern, sizeof(pattern)));
}
