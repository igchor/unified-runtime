// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: MIT

#include <cstring>
#include <uur/fixtures.h>

using urUSMHostAllocTest = uur::urQueueTest;
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urUSMHostAllocTest);

TEST_P(urUSMHostAllocTest, Success) {
    bool host_usm = false;
    size_t size = sizeof(bool);
    ASSERT_SUCCESS(urDeviceGetInfo(device, UR_DEVICE_INFO_HOST_UNIFIED_MEMORY, size, &host_usm, nullptr));
    if (!host_usm) { // Skip this test if device does not support Host USM
        GTEST_SKIP();
    }

    int *ptr = nullptr;
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_SUCCESS(urUSMHostAlloc(context, &desc, sizeof(int), 0, reinterpret_cast<void **>(&ptr)));
    ASSERT_NE(ptr, nullptr);

    // Set 0
    ur_event_handle_t event = nullptr;
    ASSERT_SUCCESS(
        urEnqueueUSMMemset(queue, ptr, 0, sizeof(int), 0, nullptr, &event));
    EXPECT_SUCCESS(urQueueFlush(queue));
    ASSERT_SUCCESS(urEventWait(1, &event));
    EXPECT_SUCCESS(urEventRelease(event));
    ASSERT_EQ(*ptr, 0);

    // Set 1, in all bytes of int
    ASSERT_SUCCESS(
        urEnqueueUSMMemset(queue, ptr, 1, sizeof(int), 0, nullptr, &event));
    EXPECT_SUCCESS(urQueueFlush(queue));
    ASSERT_SUCCESS(urEventWait(1, &event));
    EXPECT_SUCCESS(urEventRelease(event));
    // replicate it on host
    int set_data = 0;
    std::memset(&set_data, 1, sizeof(int));
    ASSERT_EQ(*ptr, set_data);

    ASSERT_SUCCESS(urUSMFree(context, ptr));
}

TEST_P(urUSMHostAllocTest, InvalidNullHandleContext) {
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    void *ptr = nullptr;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE, urUSMHostAlloc(nullptr, &desc, sizeof(int), 0, &ptr));
}

TEST_P(urUSMHostAllocTest, InvalidNullPtrFlags) {
    void *ptr = nullptr;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_POINTER, urUSMHostAlloc(context, nullptr, sizeof(int), 0, &ptr));
}

TEST_P(urUSMHostAllocTest, InvalidNullPtrMem) {
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_POINTER, urUSMHostAlloc(context, &desc, sizeof(int), 0, nullptr));
}

TEST_P(urUSMHostAllocTest, InvalidUSMSize) {
    void *ptr = nullptr;
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_USM_SIZE,
                     urUSMHostAlloc(context, &desc, 13, 0, &ptr));
}

TEST_P(urUSMHostAllocTest, InvalidValueAlignPowerOfTwo) {
    void *ptr = nullptr;
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_VALUE,
                     urUSMHostAlloc(context, &desc, sizeof(int), 1, &ptr));
}
