// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: MIT

#include <uur/fixtures.h>

using urUSMDeviceAllocTest = uur::urQueueTest;
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urUSMDeviceAllocTest);

TEST_P(urUSMDeviceAllocTest, Success) {
    void *ptr = nullptr;
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_SUCCESS(urUSMDeviceAlloc(context, device, &desc, sizeof(int), 0, &ptr));
    ASSERT_NE(ptr, nullptr);

    ur_event_handle_t event = nullptr;
    ASSERT_SUCCESS(
        urEnqueueUSMMemset(queue, ptr, 0, sizeof(int), 0, nullptr, &event));
    EXPECT_SUCCESS(urQueueFlush(queue));
    ASSERT_SUCCESS(urEventWait(1, &event));

    ASSERT_SUCCESS(urUSMFree(context, ptr));
    EXPECT_SUCCESS(urEventRelease(event));
}

TEST_P(urUSMDeviceAllocTest, InvalidNullHandleContext) {
    void *ptr = nullptr;
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE, urUSMDeviceAlloc(nullptr, device, &desc, sizeof(int), 0, &ptr));
}

TEST_P(urUSMDeviceAllocTest, InvalidNullHandleDevice) {
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    void *ptr = nullptr;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_DEVICE, urUSMDeviceAlloc(context, nullptr, &desc, sizeof(int), 0, &ptr));
}

TEST_P(urUSMDeviceAllocTest, InvalidNullPtrProps) {
    void *ptr = nullptr;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_POINTER, urUSMDeviceAlloc(context, device, nullptr, sizeof(int), 0, &ptr));
}

TEST_P(urUSMDeviceAllocTest, InvalidNullPtrResult) {
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_POINTER, urUSMDeviceAlloc(context, device, &desc, sizeof(int), 0, nullptr));
}

TEST_P(urUSMDeviceAllocTest, InvalidUSMSize) {
    void *ptr = nullptr;
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_USM_SIZE,
                     urUSMDeviceAlloc(context, device, &desc, 13, 0, &ptr));
}

TEST_P(urUSMDeviceAllocTest, InvalidValueAlignPowerOfTwo) {
    void *ptr = nullptr;
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_EQ_RESULT(
        UR_RESULT_ERROR_INVALID_VALUE,
        urUSMDeviceAlloc(context, device, &desc, sizeof(int), 1, &ptr));
}
