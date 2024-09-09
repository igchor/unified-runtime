
#include <assert.h>
#include <fstream>
#include <iostream>
#include <level_zero/ze_api.h>
#include <level_zero/zet_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#define ZE_CALL(F)                                                             \
    {                                                                          \
        auto result = F;                                                       \
        if (result != ZE_RESULT_SUCCESS) {                                     \
            printf("Error: %s -> %d\n", #F, result);                           \
            abort();                                                           \
        }                                                                      \
    }
static inline ze_driver_handle_t get_driver() {
    uint32_t drivers_num = 0;
    ZE_CALL(zeDriverGet(&drivers_num, nullptr));
    if (drivers_num == 0) {
        fprintf(stderr, "drivers_num == 0\n");
        abort();
    }
    std::vector<ze_driver_handle_t> drivers(drivers_num);
    ZE_CALL(zeDriverGet(&drivers_num, drivers.data()));
    return drivers[0];
}
static inline ze_device_handle_t get_device(ze_driver_handle_t driver) {
    uint32_t devices_num = 0;
    ZE_CALL(zeDeviceGet(driver, &devices_num, nullptr));
    if (devices_num == 0) {
        fprintf(stderr, "devices_num == 0\n");
        abort();
    }
    std::vector<ze_device_handle_t> devices(devices_num);
    ZE_CALL(zeDeviceGet(driver, &devices_num, devices.data()))
    return devices[0];
}
static inline ze_context_handle_t create_context(ze_driver_handle_t driver) {
    ze_context_desc_t ctxtDesc = {
        .stype = ZE_STRUCTURE_TYPE_CONTEXT_DESC, .pNext = NULL, .flags = 0};
    ze_context_handle_t context;
    ZE_CALL(zeContextCreate(driver, &ctxtDesc, &context));
    return context;
}
std::vector<char> load_source(const std::string &source_path) {
    std::ifstream source_file;
    source_file.open(source_path,
                     std::ios::binary | std::ios::in | std::ios::ate);
    if (!source_file.is_open()) {
        throw std::runtime_error("Cannot open binary file: " + source_path);
    }
    size_t source_size = static_cast<size_t>(source_file.tellg());
    source_file.seekg(0, std::ios::beg);
    std::vector<char> device_binary(source_size);
    source_file.read(device_binary.data(), source_size);
    if (!source_file) {
        source_file.close();
        throw std::runtime_error(
            "failed reading kernel source data from file: " + source_path);
    }
    source_file.close();
    return device_binary;
}
void init_level_zero() {
    ze_result_t result = zeInit(ZE_INIT_FLAG_GPU_ONLY);
    if (result != ZE_RESULT_SUCCESS) {
        fprintf(stderr, "zeInit() failed!\n");
        abort();
    }
}

void run_kernel(ze_context_handle_t context, ze_device_handle_t device,
                std::vector<char> &spir1) {
    ze_event_pool_counter_based_exp_desc_t counterBasedExt = {
        ZE_STRUCTURE_TYPE_COUNTER_BASED_EVENT_POOL_EXP_DESC, nullptr};
    counterBasedExt.flags = ZE_EVENT_POOL_COUNTER_BASED_EXP_FLAG_IMMEDIATE;

    ze_event_pool_desc_t eventPoolDesc = {
        .stype = ZE_STRUCTURE_TYPE_EVENT_POOL_DESC,
        .pNext = &counterBasedExt,
        .flags = ZE_EVENT_POOL_FLAG_HOST_VISIBLE,
        .count = 64};
    ze_event_pool_handle_t eventPool1;
    ZE_CALL(
        zeEventPoolCreate(context, &eventPoolDesc, 1, &device, &eventPool1));

    ze_event_handle_t copyEvent;
    ze_event_handle_t computeEvent;

    // Create a bunch of events - this simulates what L0 v2 adapters is
    // doing right now.
    for (int i = 0; i < 62; i++) {
        ze_event_desc_t eventDesc = {.stype = ZE_STRUCTURE_TYPE_EVENT_DESC,
                                     .pNext = NULL,
                                     .signal = ZE_EVENT_SCOPE_FLAG_HOST,
                                     .wait = 0};
        eventDesc.index = i;
        ze_event_handle_t leakHandle;
        ZE_CALL(zeEventCreate(eventPool1, &eventDesc, &leakHandle));
    }
    ze_event_desc_t eventDesc = {.stype = ZE_STRUCTURE_TYPE_EVENT_DESC,
                                 .pNext = NULL,
                                 .signal = ZE_EVENT_SCOPE_FLAG_HOST,
                                 .wait = 0};
    eventDesc.index = 62;
    ZE_CALL(zeEventCreate(eventPool1, &eventDesc, &copyEvent));
    eventDesc.index = 63;
    ZE_CALL(zeEventCreate(eventPool1, &eventDesc, &computeEvent));

    ze_device_mem_alloc_desc_t deviceDesc = {
        .stype = ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC,
        .pNext = NULL,
        .flags = 0,
        .ordinal = 0};
    ze_host_mem_alloc_desc_t hostDesc = {
        .stype = ZE_STRUCTURE_TYPE_HOST_MEM_ALLOC_DESC,
        .pNext = NULL,
        .flags = 0};

    std::vector<void *> ptrs(50);

    static constexpr uint32_t copyOrdinal = 1;
    static constexpr uint32_t computeOrdinal = 0;
    ze_command_queue_desc_t cmdQueueDesc = {
        .stype = ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC,
        .pNext = NULL,
        .ordinal = copyOrdinal,
        .index = 0,
        .flags = ZE_COMMAND_QUEUE_FLAG_IN_ORDER,
        .mode = ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS,
        .priority = ZE_COMMAND_QUEUE_PRIORITY_NORMAL};
    ze_command_list_handle_t copyCmdList;
    ZE_CALL(zeCommandListCreateImmediate(context, device, &cmdQueueDesc,
                                         &copyCmdList));
    ze_command_queue_desc_t computeCmdQueueDesc = {
        .stype = ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC,
        .pNext = NULL,
        .ordinal = computeOrdinal,
        .index = 0,
        .flags = ZE_COMMAND_QUEUE_FLAG_IN_ORDER,
        .mode = ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS,
        .priority = ZE_COMMAND_QUEUE_PRIORITY_NORMAL};
    ze_command_list_handle_t computeCmdList;
    ZE_CALL(zeCommandListCreateImmediate(context, device, &computeCmdQueueDesc,
                                         &computeCmdList));
    constexpr int length = 100;
    uint32_t data1[length];
    for (int i = 0; i < length; i++) {
        data1[i] = 1;
    }

    // Create ptrs.size() memory allocations and the same number of kernels
    // Each kernel will operate on a different memory allocation
    std::vector<ze_kernel_handle_t> kernels;
    for (int i = 0; i < ptrs.size(); i++) {
        // zeModuleCreate
        ze_module_desc_t moduleDesc = {
            .stype = ZE_STRUCTURE_TYPE_MODULE_DESC,
            .pNext = nullptr,
            .format = ZE_MODULE_FORMAT_IL_SPIRV,
            .inputSize = spir1.size(),
            .pInputModule = reinterpret_cast<const uint8_t *>(spir1.data()),
            .pBuildFlags = ""};
        ze_module_handle_t module;
        ZE_CALL(zeModuleCreate(context, device, &moduleDesc, &module, nullptr));
        ze_kernel_desc_t kernelDesc = {
            .stype = ZE_STRUCTURE_TYPE_KERNEL_DESC,
            .pNext = nullptr,
            .pKernelName =
                "_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_E12IncrementBy1"};
        ze_kernel_handle_t kernel;
        ZE_CALL(zeKernelCreate(module, &kernelDesc, &kernel));
        ZE_CALL(zeMemAllocShared(context, &deviceDesc, &hostDesc, 400, 0,
                                 device, &ptrs[i]));
        static constexpr uint32_t initial_value = 1;
        // ZE_CALL(zeCommandListAppendMemoryFill(copyCmdList, ptrs[i], &initial_value, sizeof(initial_value), 400, copyEvent, 0, nullptr));
        ZE_CALL(zeCommandListAppendMemoryCopy(copyCmdList, ptrs[i], data1,
                                              length * sizeof(uint32_t),
                                              copyEvent, 0, nullptr));
        ZE_CALL(zeCommandListHostSynchronize(copyCmdList, UINT64_MAX));
        ZE_CALL(zeKernelSetArgumentValue(kernel, 0, 8, &ptrs[i]));
        kernels.push_back(kernel);
    }

    // Execute the first kernel (which will increment every element in it's allocation by 1),
    // then, copy the memory to the next allocation, and launch the next kernel.
    // This will be repeated ptrs.size() times.
    ze_group_count_t launchFuncArgs = {1, 1, 1};
    auto kernel = kernels[0];
    uint32_t groupSizeX, groupSizeY, groupSizeZ;
    ZE_CALL(zeKernelSuggestGroupSize(kernel, 100, 1, 1, &groupSizeX,
                                     &groupSizeY, &groupSizeZ));
    // zeKernelSetGroupSize
    ZE_CALL(zeKernelSetGroupSize(kernel, groupSizeX, groupSizeY, groupSizeZ));
    ZE_CALL(zeCommandListAppendLaunchKernel(
        computeCmdList, kernels[0], &launchFuncArgs, computeEvent, 0, nullptr));
    ZE_CALL(zeCommandListAppendMemoryCopy(copyCmdList, ptrs[1], ptrs[0],
                                          length * sizeof(uint32_t), copyEvent,
                                          1, &computeEvent));
    for (int i = 1; i < ptrs.size(); i++) {
        auto kernel = kernels[i];
        uint32_t groupSizeX, groupSizeY, groupSizeZ;
        ZE_CALL(zeKernelSuggestGroupSize(kernel, 100, 1, 1, &groupSizeX,
                                         &groupSizeY, &groupSizeZ));
        // zeKernelSetGroupSize
        ZE_CALL(
            zeKernelSetGroupSize(kernel, groupSizeX, groupSizeY, groupSizeZ));
        ZE_CALL(zeCommandListAppendLaunchKernel(computeCmdList, kernels[i],
                                                &launchFuncArgs, computeEvent,
                                                1, &copyEvent));
        if (i < ptrs.size() - 1) {
            ZE_CALL(zeCommandListAppendMemoryCopy(
                copyCmdList, ptrs[i + 1], ptrs[i], length * sizeof(uint32_t),
                copyEvent, 1, &computeEvent));
        }
    }
    ZE_CALL(zeCommandListHostSynchronize(computeCmdList, UINT64_MAX));

    // After all operations are done, each allocation should
    // contain the value 1 + i, where i is the index of the allocation
    size_t ExpectedValue = 1;
    for (size_t i = 0; i < ptrs.size(); i++) {
        ExpectedValue++;
        for (uint32_t j = 0; j < 100; ++j) {
            if (reinterpret_cast<uint32_t *>(ptrs[i])[j] != ExpectedValue) {
                throw std::runtime_error(
                    "Expected: " + std::to_string(ExpectedValue) + " Got: " +
                    std::to_string(reinterpret_cast<uint32_t *>(ptrs[i])[j]));
            }
        }
    }
}
int main(int argc, char *argv[]) {
    init_level_zero();
    auto driver = get_driver();
    auto context = create_context(driver);
    auto device = get_device(driver);
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " "
                  << "1.spv " << std::endl;
        return -1;
    }
    auto k1 = argv[1];
    auto spir1 = load_source(k1);
    run_kernel(context, device, spir1);
    return 0;
}
