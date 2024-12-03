// Copyright (C) 2024 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <ze_api.h>

#include "uur/fixtures.h"

struct urEnqueueKernelLaunchTest : uur::urKernelExecutionTest {
    void SetUp() override {
        program_name = "fill";
        UUR_RETURN_ON_FATAL_FAILURE(urKernelExecutionTest::SetUp());
    }

    uint32_t val = 42;
    size_t global_size = 32;
    size_t global_offset = 0;
    size_t n_dimensions = 1;
};
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urEnqueueKernelLaunchTest);

TEST_P(urEnqueueKernelLaunchTest, DeferredKernelRelease) {
    ur_mem_handle_t buffer = nullptr;
    AddBuffer1DArg(sizeof(val) * global_size, &buffer);
    AddPodArg(val);

    ur_event_handle_t event = nullptr;
    ASSERT_SUCCESS(urEventCreateWithNativeHandle(
        reinterpret_cast<ur_native_handle_t>(nullptr), context, nullptr,
        &event));

    ASSERT_SUCCESS(urEnqueueEventsWait(queue, 1, &event, nullptr));
    ASSERT_SUCCESS(urEnqueueKernelLaunch(queue, kernel, n_dimensions,
                                         &global_offset, &global_size, nullptr,
                                         0, nullptr, nullptr));
    ASSERT_SUCCESS(urKernelRelease(kernel));

    // Kernel should still be alive since kernel launch is pending
    ur_context_handle_t contextFromKernel;
    ASSERT_SUCCESS(urKernelGetInfo(kernel, UR_KERNEL_INFO_CONTEXT,
                                   sizeof(ur_context_handle_t),
                                   &contextFromKernel, nullptr));

    ASSERT_EQ(context, contextFromKernel);

    ze_event_handle_t ze_event = nullptr;
    ASSERT_SUCCESS(urEventGetNativeHandle(
        event, reinterpret_cast<ur_native_handle_t *>(&ze_event)));
    ASSERT_EQ(zeEventHostSignal(ze_event), ZE_RESULT_SUCCESS);

    ASSERT_SUCCESS(urQueueFinish(queue));

    kernel = nullptr;

    ASSERT_SUCCESS(urEventRelease(event));
}
