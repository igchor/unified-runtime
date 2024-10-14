// Copyright (C) 2023 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <uur/fixtures.h>
#include <uur/raii.h>

using urMemBufferCreateWithNativeHandleTest = uur::urMemBufferTest;
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urMemBufferCreateWithNativeHandleTest);

TEST_P(urMemBufferCreateWithNativeHandleTest, Success) {
    ur_native_handle_t hNativeMem = 0;
    {
        UUR_ASSERT_SUCCESS_OR_UNSUPPORTED(
            urMemGetNativeHandle(buffer, device, &hNativeMem));
    }

    // We cannot assume anything about a native_handle, not even if it's
    // `nullptr` since this could be a valid representation within a backend.
    // We can however convert the native_handle back into a unified-runtime handle
    // and perform some query on it to verify that it works.
    ur_mem_handle_t mem = nullptr;
    ur_mem_native_properties_t props = {
        /*.stype =*/UR_STRUCTURE_TYPE_MEM_NATIVE_PROPERTIES,
        /*.pNext =*/nullptr,
        /*.isNativeHandleOwned =*/false,
    };
    UUR_ASSERT_SUCCESS_OR_UNSUPPORTED(
        urMemBufferCreateWithNativeHandle(hNativeMem, context, &props, &mem));
    ASSERT_NE(mem, nullptr);

    size_t alloc_size = 0;
    ASSERT_SUCCESS(urMemGetInfo(mem, UR_MEM_INFO_SIZE, sizeof(size_t),
                                &alloc_size, nullptr));

    ASSERT_SUCCESS(urMemRelease(mem));
}

struct MultiQueueMemBufferTest : uur::urMultiQueueTest {
    void SetUp() override {
        UUR_RETURN_ON_FATAL_FAILURE(urMultiQueueTest::SetUp());
        ASSERT_SUCCESS(urMemBufferCreate(context, UR_MEM_FLAG_READ_WRITE,
                                         buffer_size, nullptr, &buffer));
        ASSERT_NE(nullptr, buffer);
    }

    void TearDown() override {
        if (buffer) {
            EXPECT_SUCCESS(urMemRelease(buffer));
        }
        UUR_RETURN_ON_FATAL_FAILURE(urMultiQueueTest::TearDown());
    }

    const size_t buffer_size = 4096;
    ur_mem_handle_t buffer = nullptr;
};

TEST_F(MultiQueueMemBufferTest, WriteBack) {
    ur_native_handle_t hNativeMem = 0;
    {
        UUR_ASSERT_SUCCESS_OR_UNSUPPORTED(
            urMemGetNativeHandle(buffer, device, &hNativeMem));
    }

    ur_mem_handle_t mem = nullptr;
    ur_mem_native_properties_t props = {
        /*.stype =*/UR_STRUCTURE_TYPE_MEM_NATIVE_PROPERTIES,
        /*.pNext =*/nullptr,
        /*.isNativeHandleOwned =*/false,
    };
    {
        UUR_ASSERT_SUCCESS_OR_UNSUPPORTED(urMemBufferCreateWithNativeHandle(
            hNativeMem, context, &props, &mem));
    }
    ASSERT_NE(mem, nullptr);

    const uint8_t pattern = 0x11;
    std::vector<uint8_t> src(buffer_size, pattern);

    // write data to the buffer and destroy the buffer
    ASSERT_SUCCESS(urEnqueueMemBufferWrite(queue2, mem, true, 0, buffer_size,
                                           src.data(), 0, nullptr, nullptr));
    ASSERT_SUCCESS(urMemRelease(mem));

    // Create the buffer again and read back the data, data should have been written to the
    // memory behind the native handle. Use different queue to test data migration logic.
    ASSERT_SUCCESS(
        urMemBufferCreateWithNativeHandle(hNativeMem, context, &props, &mem));
    ASSERT_NE(mem, nullptr);

    std::vector<uint8_t> dst(buffer_size, 0);
    ASSERT_SUCCESS(urEnqueueMemBufferRead(queue1, mem, true, 0, buffer_size,
                                          dst.data(), 0, nullptr, nullptr));

    ASSERT_EQ(src, dst);

    ASSERT_SUCCESS(urMemRelease(mem));
}
