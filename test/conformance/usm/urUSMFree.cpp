// Copyright (C) 2023 Intel Corporation
// SPDX-License-Identifier: MIT

#include <uur/fixtures.h>

using urUSMFreeTest = uur::urQueueTest;
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urUSMFreeTest);

TEST_P(urUSMFreeTest, Success) {
    void *ptr = nullptr;
    ur_usm_desc_t desc = {};
    desc.stype = UR_STRUCTURE_TYPE_USM_DESC;
    ASSERT_SUCCESS(urUSMDeviceAlloc(context, device, &desc, sizeof(int), 0, &ptr));

    ur_event_handle_t event = nullptr;
    ASSERT_SUCCESS(
        urEnqueueUSMMemset(queue, ptr, 0, sizeof(int), 0, nullptr, &event));
    EXPECT_SUCCESS(urQueueFlush(queue));
    ASSERT_SUCCESS(urEventWait(1, &event));

    ASSERT_NE(ptr, nullptr);
    ASSERT_SUCCESS(urUSMFree(context, ptr));
}

TEST_P(urUSMFreeTest, InvalidNullContext) { ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_HANDLE, urUSMFree(nullptr, nullptr)); }

TEST_P(urUSMFreeTest, InvalidNullPtrMem) { ASSERT_EQ_RESULT(UR_RESULT_ERROR_INVALID_NULL_POINTER, urUSMFree(context, nullptr)); }
