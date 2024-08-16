// Copyright (C) 2024 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <uur/fixtures.h>
#include <uur/raii.h>

#include <thread>
#include <utility>

struct urMultiDeviceKernels {
    std::string KernelName;
    std::vector<ur_program_handle_t> programs;
    std::vector<ur_kernel_handle_t> kernels;
    std::vector<void *> SharedMem;

    static constexpr char ProgramName[] = "multiply";
    static constexpr size_t ArraySize = 100;
    static constexpr size_t InitialValue = 10;

    void SetUp(ur_platform_handle_t platform, ur_context_handle_t context, std::vector<ur_queue_handle_t> queues, std::vector<ur_device_handle_t> devices) {
        programs.resize(devices.size());
        kernels.resize(devices.size());
        SharedMem.resize(devices.size());

        KernelName = uur::KernelsEnvironment::instance->GetEntryPointNames(
            ProgramName)[0];

        std::shared_ptr<std::vector<char>> il_binary;
        std::vector<ur_program_metadata_t> metadatas{};

        uur::KernelsEnvironment::instance->LoadSource(ProgramName, il_binary);

        for (size_t i = 0; i < devices.size(); i++) {
            const ur_program_properties_t properties = {
                UR_STRUCTURE_TYPE_PROGRAM_PROPERTIES, nullptr,
                static_cast<uint32_t>(metadatas.size()),
                metadatas.empty() ? nullptr : metadatas.data()};

            uur::raii::Program program;
            ASSERT_SUCCESS(uur::KernelsEnvironment::instance->CreateProgram(
                platform, context, devices[i], *il_binary, &properties,
                &programs[i]));

            ASSERT_SUCCESS(urProgramBuildExp(programs[i], devices.size(),
                                             devices.data(), nullptr));
            ASSERT_SUCCESS(urKernelCreate(programs[i], KernelName.data(),
                                          &kernels[i]));

            ASSERT_SUCCESS(
                urUSMSharedAlloc(context, devices[i], nullptr, nullptr,
                                 ArraySize * sizeof(uint32_t), &SharedMem[i]));

            ASSERT_SUCCESS(urEnqueueUSMFill(queues[i], SharedMem[i],
                                            sizeof(uint32_t), &InitialValue,
                                            ArraySize * sizeof(uint32_t), 0,
                                            nullptr, nullptr /* &Event */));
            ASSERT_SUCCESS(urQueueFinish(queues[i]));

            ASSERT_SUCCESS(urKernelSetArgPointer(kernels[i], 0, nullptr,
                                                 SharedMem[i]));
        }
    }

    void TearDown(ur_context_handle_t context) {
        for (auto &Ptr : SharedMem) {
            urUSMFree(context, Ptr);
        }
        for (const auto &kernel : kernels) {
            EXPECT_SUCCESS(urKernelRelease(kernel));
        }
        for (const auto &program : programs) {
            EXPECT_SUCCESS(urProgramRelease(program));
        }
    }
};

struct urEnqueueKernelLaunchMultiplyMultiDeviceTest
    : public uur::urMultiDeviceContextTestTemplate<1> // allow a single device for this tests
    {
    using uur::urMultiDeviceContextTestTemplate<1>::context;

    static constexpr size_t duplicateDevices = 4;

    std::vector<ur_queue_handle_t> queues;
    std::vector<ur_device_handle_t> devices;

    urMultiDeviceKernels kernels;

    void SetUp() override {
        UUR_RETURN_ON_FATAL_FAILURE(uur::urMultiDeviceContextTestTemplate<1>::SetUp());

        devices = uur::KernelsEnvironment::instance->devices;
        auto originalQueues = queues;

        for (size_t i = 0; i < duplicateDevices; i++) {
            devices.insert(devices.end(),
                           uur::KernelsEnvironment::instance->devices.begin(),
                           uur::KernelsEnvironment::instance->devices.end());
        }

        for (auto &device : devices) {
            ur_queue_handle_t queue = nullptr;
            ASSERT_SUCCESS(urQueueCreate(context, device, 0, &queue));
            queues.push_back(queue);
        }

        kernels.SetUp(platform, context, queues, devices);
    }

    void TearDown() override {
        kernels.TearDown(context);
        for (const auto &queue : queues) {
            EXPECT_SUCCESS(urQueueRelease(queue));
        }
        UUR_RETURN_ON_FATAL_FAILURE(uur::urMultiDeviceContextTestTemplate<1>::TearDown());
    }
};

struct urEnqueueKernelLaunchMultiplyTest : uur::urQueueTestWithParam<uur::BoolTestParam> {
    urMultiDeviceKernels kernels;
    static constexpr size_t numOps = 20;

    void SetUp() override {
        UUR_RETURN_ON_FATAL_FAILURE(uur::urQueueTestWithParam<uur::BoolTestParam>::SetUp());
        std::vector<ur_queue_handle_t> queues;
        std::vector<ur_device_handle_t> devices;

        // use the same queue and device for all operations
        for (size_t i = 0; i < numOps; i++) {
            queues.push_back(queue);
            devices.push_back(device);
        }

        kernels.SetUp(platform, context, queues, devices);
    }

    void TearDown() override {
        kernels.TearDown(context);
        UUR_RETURN_ON_FATAL_FAILURE(uur::urQueueTestWithParam<uur::BoolTestParam>::TearDown());
    }
};

UUR_TEST_SUITE_P(
    urEnqueueKernelLaunchMultiplyTest,
    testing::ValuesIn(uur::BoolTestParam::makeBoolParam("UseEvents")),
    uur::deviceTestWithParamPrinter<uur::BoolTestParam>);

TEST_P(urEnqueueKernelLaunchMultiplyTest, Success) {
    constexpr size_t global_offset = 0;
    constexpr size_t n_dimensions = 1;

    auto useEvents = std::get<1>(GetParam()).value;

    std::vector<uur::raii::Event> Events(numOps * 2);
    for (size_t i = 0; i < numOps; i++) {
        size_t waitNum = 0;
        ur_event_handle_t *lastEvent = nullptr;
        ur_event_handle_t *kernelEvent = nullptr;
        ur_event_handle_t *memcpyEvent = nullptr;

        if (useEvents) {
            // Events are: kernelEvent0, memcpyEvent0, kernelEvent1, ...
            waitNum = i > 0 ? 1 : 0;
            lastEvent = i > 0 ? Events[i * 2 - 1].ptr() : nullptr;

            kernelEvent = Events[i * 2].ptr();
            memcpyEvent = Events[i * 2 + 1].ptr();
        }

        // execute kernel that multiplies each element by 2
        ASSERT_SUCCESS(urEnqueueKernelLaunch(
            queue, kernels.kernels[i], n_dimensions, &global_offset,
            &urMultiDeviceKernels::ArraySize, nullptr, waitNum, lastEvent, kernelEvent));

        // copy the memory to next device
        if (i < numOps - 1) {
            ASSERT_SUCCESS(
                urEnqueueUSMMemcpy(queue, false, kernels.SharedMem[i + 1],
                                   kernels.SharedMem[i], urMultiDeviceKernels::ArraySize * sizeof(uint32_t),
                                   useEvents, kernelEvent, memcpyEvent));
        }
    }

    if (useEvents) {
        // TODO: just wait on the last event, once urEventWait is implemented
        urQueueFinish(queue);
    } else {
        urQueueFinish(queue);
    }

    size_t ExpectedValue = urMultiDeviceKernels::InitialValue;
    for (size_t i = 0; i < numOps; i++) {
        ExpectedValue *= 2;
        for (uint32_t j = 0; j < urMultiDeviceKernels::ArraySize; ++j) {
            ASSERT_EQ(reinterpret_cast<uint32_t *>(kernels.SharedMem[i])[j],
                      ExpectedValue);
        }
    }
}

// Do a chain of kernelLaunch(dev0) -> memcpy(dev0, dev1) -> kernelLaunch(dev1) ... ops
TEST_F(urEnqueueKernelLaunchMultiplyMultiDeviceTest, Success) {
    size_t returned_size;
    ASSERT_SUCCESS(urDeviceGetInfo(devices[0], UR_DEVICE_INFO_EXTENSIONS, 0,
                                    nullptr, &returned_size));

    std::unique_ptr<char[]> returned_extensions(new char[returned_size]);

    ASSERT_SUCCESS(urDeviceGetInfo(devices[0], UR_DEVICE_INFO_EXTENSIONS,
                                    returned_size, returned_extensions.get(),
                                    nullptr));

    std::string_view extensions_string(returned_extensions.get());
    const bool usm_p2p_support =
        extensions_string.find(UR_USM_P2P_EXTENSION_STRING_EXP) !=
        std::string::npos;

    if (!usm_p2p_support) {
        GTEST_SKIP() << "EXP usm p2p feature is not supported.";
    }

    constexpr size_t global_offset = 0;
    constexpr size_t n_dimensions = 1;

    std::vector<uur::raii::Event> Events(devices.size() * 2);
    for (size_t i = 0; i < devices.size(); i++) {
        // Events are: kernelEvent0, memcpyEvent0, kernelEvent1, ...
        size_t waitNum = i > 0 ? 1 : 0;
        ur_event_handle_t * lastEvent = i > 0 ? Events[i * 2 - 1].ptr() : nullptr;
        ur_event_handle_t * kernelEvent = Events[i * 2].ptr();
        ur_event_handle_t * memcpyEvent = Events[i * 2 + 1].ptr();

        // execute kernel that multiplies each element by 2
        ASSERT_SUCCESS(urEnqueueKernelLaunch(
            queues[i], kernels.kernels[i], n_dimensions, &global_offset,
            &urMultiDeviceKernels::ArraySize, nullptr, waitNum, lastEvent, kernelEvent));

        // copy the memory to next device
        if (i < devices.size() - 1) {
            ASSERT_SUCCESS(
                urEnqueueUSMMemcpy(queues[i], false, kernels.SharedMem[i + 1],
                                   kernels.SharedMem[i], urMultiDeviceKernels::ArraySize * sizeof(uint32_t),
                                   1, kernelEvent, memcpyEvent));
        }
    }

    // synchronize on the last queue only, this has to ensure all the operations
    // are completed
    urQueueFinish(queues.back());

    size_t ExpectedValue = urMultiDeviceKernels::InitialValue;
    for (size_t i = 0; i < devices.size(); i++) {
        ExpectedValue *= 2;
        for (uint32_t j = 0; j < urMultiDeviceKernels::ArraySize; ++j) {
            ASSERT_EQ(reinterpret_cast<uint32_t *>(kernels.SharedMem[i])[j],
                      ExpectedValue);
        }
    }
}

template <typename T>
inline std::string
printParams(const testing::TestParamInfo<typename T::ParamType> &info) {
    std::stringstream ss;

    auto param1 = std::get<0>(info.param);
    auto param2 = std::get<1>(info.param);
    auto param3 = std::get<2>(info.param);

    ss << (param1.value ? "" : "No") << param1.name;
    ss << (param2.value ? "" : "No") << param2.name;
    ss << (param3.value ? "" : "No") << param3.name;

    return ss.str();
}

struct urEnqueueKernelLaunchMultiplyMultiDeviceMultiThreadTest : urEnqueueKernelLaunchMultiplyMultiDeviceTest, testing::WithParamInterface<std::tuple<uur::BoolTestParam, uur::BoolTestParam, uur::BoolTestParam>> {
    using Param = std::tuple<uur::BoolTestParam, uur::BoolTestParam, uur::BoolTestParam>;
};

INSTANTIATE_TEST_SUITE_P(,
    urEnqueueKernelLaunchMultiplyMultiDeviceMultiThreadTest,
        testing::Combine(
            testing::ValuesIn(uur::BoolTestParam::makeBoolParam("UseEvents")),
            testing::ValuesIn(uur::BoolTestParam::makeBoolParam("QueuePerThread")),
            testing::ValuesIn(uur::BoolTestParam::makeBoolParam("KernelPerThread"))
        ),
    printParams<urEnqueueKernelLaunchMultiplyMultiDeviceMultiThreadTest>
);

// Enqueue kernelLaunch concurrently from multiple threads
// With !queuePerThread this becomes a test on a single device
TEST_P(urEnqueueKernelLaunchMultiplyMultiDeviceMultiThreadTest, Success) {
    size_t numThreads = devices.size();
    std::vector<std::thread> threads;

    static constexpr size_t numOpsPerThread = 3;

    auto useEvents = std::get<0>(GetParam()).value;
    auto queuePerThread = std::get<1>(GetParam()).value;
    auto kernelPerThread = std::get<2>(GetParam()).value;

    // TODO: change the value in the kernel to uint64_t
    if (pow(2, numThreads * numOpsPerThread) > std::numeric_limits<uint32_t>::max()) {
        GTEST_SKIP() << "Computed value would exceed uint32_t max";
    }

    for (size_t i = 0; i < numThreads; i++) {
        threads.emplace_back([this, i, queuePerThread, useEvents, kernelPerThread]() {
            constexpr size_t global_offset = 0;
            constexpr size_t n_dimensions = 1;

            auto queue = queuePerThread ? queues[i] : queues.back();
            auto kernel = kernelPerThread ? kernels.kernels[i] : kernels.kernels.back();
            auto sharedPtr = kernelPerThread ? kernels.SharedMem[i] : kernels.SharedMem.back();

            std::vector<uur::raii::Event> Events(numOpsPerThread + 1);
            for (size_t j = 0; j < numOpsPerThread; j++) {
                size_t waitNum = 0;
                ur_event_handle_t * lastEvent = nullptr;
                ur_event_handle_t * signalEvent = nullptr;

                if (useEvents) {
                    waitNum = j > 0 ? 1 : 0;
                    lastEvent = j > 0 ? Events[j - 1].ptr() : nullptr;
                    signalEvent = Events[j].ptr();
                }

                // execute kernel that multiplies each element by 2
                ASSERT_SUCCESS(urEnqueueKernelLaunch(
                    queue, kernel, n_dimensions, &global_offset,
                    &urMultiDeviceKernels::ArraySize, nullptr, waitNum, lastEvent, signalEvent));
            }

            std::vector<uint32_t> data(urMultiDeviceKernels::ArraySize);

            auto lastEvent = useEvents ? Events[numOpsPerThread - 1].ptr() : nullptr;
            auto signalEvent = useEvents ? Events.back().ptr() : nullptr;
            ASSERT_SUCCESS(
                urEnqueueUSMMemcpy(queue, false, data.data(),
                                   sharedPtr, urMultiDeviceKernels::ArraySize * sizeof(uint32_t),
                                   useEvents, lastEvent, signalEvent));

            urQueueFinish(queue);
            // TODO: when useEvents, wait on event instead

            size_t ExpectedValue = urMultiDeviceKernels::InitialValue;
                ExpectedValue *= pow(2, numOpsPerThread);
                for (uint32_t j = 0; j < urMultiDeviceKernels::ArraySize; ++j) {
                    if (kernelPerThread) {
                        ASSERT_EQ(data[j], ExpectedValue);
                    } else {
                        // All threads write to the same memory, so the value might be greater
                        ASSERT_GE(data[j], ExpectedValue);
                    }
                }
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }

    if (!kernelPerThread) {
        size_t ExpectedValue = urMultiDeviceKernels::InitialValue;
        ExpectedValue *= pow(2, numOpsPerThread * numThreads);

        for (size_t i = 0; i < devices.size(); i++) {
            for (uint32_t j = 0; j < urMultiDeviceKernels::ArraySize; ++j) {
                ASSERT_EQ(reinterpret_cast<uint32_t *>(kernels.SharedMem.back())[j],
                        ExpectedValue);
            }
        }
    }
}
