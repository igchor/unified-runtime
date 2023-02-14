// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: MIT

#include <uur/fixtures.h>

using urUSMSharedAllocTest = uur::urQueueTest;
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urUSMSharedAllocTest);

TEST_P(urUSMSharedAllocTest, Success) {
    void *ptr = nullptr;
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_SUCCESS(urUSMSharedAlloc(context, device, &desc, sizeof(int), 0, &ptr));

    ur_event_handle_t event = nullptr;
    ASSERT_SUCCESS(urEnqueueUSMMemset(queue, ptr, 0, sizeof(int), 0, nullptr, &event));
    ASSERT_SUCCESS(urEventWait(1, &event));

    ASSERT_SUCCESS(urUSMFree(context, ptr));
    EXPECT_SUCCESS(urEventRelease(event));
}

TEST_P(urUSMSharedAllocTest, InvalidNullHandleContext) {
    void *ptr = nullptr;
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE, urUSMSharedAlloc(nullptr, device, &desc, sizeof(int), 0, &ptr));
}

TEST_P(urUSMSharedAllocTest, InvalidNullHandleDevice) {
    void *ptr = nullptr;
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE, urUSMSharedAlloc(context, nullptr, &desc, sizeof(int), 0, &ptr));
}

TEST_P(urUSMSharedAllocTest, InvalidNullPtrFlags) {
    void *ptr = nullptr;
    ASSERT_EQ_RESULT(
        UR_RESULT_ERROR_INVALID_NULL_POINTER,
        urUSMSharedAlloc(context, device, nullptr, sizeof(int), 0, &ptr));
}

TEST_P(urUSMSharedAllocTest, InvalidNullPtrMem) {
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE, urUSMSharedAlloc(context, device, &desc, sizeof(int), 0, nullptr));
}

TEST_P(urUSMSharedAllocTest, InvalidUSMSize) {
    void *ptr = nullptr;
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_USM_SIZE,
                     urUSMSharedAlloc(context, device, &desc, 13, 0, &ptr));
}

TEST_P(urUSMSharedAllocTest, InvalidValueAlignPowerOfTwo) {
    void *ptr = nullptr;
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_EQ_RESULT(
        UR_RESULT_ERROR_INVALID_VALUE,
        urUSMSharedAlloc(context, device, &desc, sizeof(int), 1, &ptr));
}
