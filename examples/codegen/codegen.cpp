/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
 * See LICENSE.TXT
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <iostream>
#include <vector>

#include <dlfcn.h>

#include "helpers.h"
#include "ur_api.h"

constexpr unsigned PAGE_SIZE = 4096;

static ur_result_t (*urInitFn)(ur_device_init_flags_t,
                               ur_loader_config_handle_t);
static ur_result_t (*urAdapterGetFn)(uint32_t, ur_adapter_handle_t *,
                                     uint32_t *);
static ur_result_t (*urPlatformGetFn)(ur_adapter_handle_t *, uint32_t, uint32_t,
                                      ur_platform_handle_t *, uint32_t *);
static ur_result_t (*urDeviceGetFn)(ur_platform_handle_t, ur_device_type_t,
                                    uint32_t, ur_device_handle_t *, uint32_t *);
static ur_result_t (*urContextGetInfoFn)(ur_context_handle_t, ur_context_info_t,
                                         size_t, void *, size_t *);
static ur_result_t (*urContextCreateFn)(uint32_t, const ur_device_handle_t *,
                                        const ur_context_properties_t *,
                                        ur_context_handle_t *);
static ur_result_t (*urTearDownFn)(void* params);
static ur_result_t (*urAdapterGetInfoFn)(
    ur_adapter_handle_t hAdapter, ///< [in] handle of the adapter
    ur_adapter_info_t propName,   ///< [in] type of the info to retrieve
    size_t propSize,              ///< [in] the number of bytes pointed to by pPropValue.
    void *pPropValue,             ///< [out][optional][typename(propName, propSize)] array of bytes holding
                                  ///< the info.
                                  ///< If Size is not equal to or greater to the real number of bytes needed
                                  ///< to return the info then the ::UR_RESULT_ERROR_INVALID_SIZE error is
                                  ///< returned and pPropValue is not used.
    size_t *pPropSizeRet          ///< [out][optional] pointer to the actual number of bytes being queried by pPropValue.
);

static ur_result_t (*urInitFn2)(ur_device_init_flags_t,
                               ur_loader_config_handle_t);
static ur_result_t (*urContextGetInfoFn2)(ur_context_handle_t, ur_context_info_t,
                                         size_t, void *, size_t *);
static ur_result_t (*urAdapterGetFn2)(uint32_t, ur_adapter_handle_t *,
                                     uint32_t *);

void ur_check(const ur_result_t r) {
    if (r != UR_RESULT_SUCCESS) {
        urTearDownFn(nullptr);
        throw std::runtime_error("Unified runtime error: " + std::to_string(r));
    }
}

std::vector<ur_adapter_handle_t> get_adapters() {
    uint32_t adapterCount = 0;
    ur_check(urAdapterGetFn(0, nullptr, &adapterCount));

    if (!adapterCount) {
        throw std::runtime_error("No adapters available.");
    }

    std::vector<ur_adapter_handle_t> adapters(adapterCount);
    ur_check(urAdapterGetFn(adapterCount, adapters.data(), nullptr));
    return adapters;
}

std::vector<ur_adapter_handle_t>
get_supported_adapters(std::vector<ur_adapter_handle_t> &adapters) {
    std::vector<ur_adapter_handle_t> supported_adapters;
    for (auto adapter : adapters) {
        ur_adapter_backend_t backend;
        ur_check(urAdapterGetInfoFn(adapter, UR_ADAPTER_INFO_BACKEND,
                                  sizeof(ur_adapter_backend_t), &backend,
                                  nullptr));

        if (backend == UR_ADAPTER_BACKEND_LEVEL_ZERO) {
            supported_adapters.push_back(adapter);
        }
    }

    return supported_adapters;
}

std::vector<ur_platform_handle_t>
get_platforms(std::vector<ur_adapter_handle_t> &adapters) {
    uint32_t platformCount = 0;
    ur_check(urPlatformGetFn(adapters.data(), adapters.size(), 1, nullptr,
                           &platformCount));

    if (!platformCount) {
        throw std::runtime_error("No platforms available.");
    }

    std::vector<ur_platform_handle_t> platforms(platformCount);
    ur_check(urPlatformGetFn(adapters.data(), adapters.size(), platformCount,
                           platforms.data(), nullptr));
    return platforms;
}

std::vector<ur_device_handle_t> get_gpus(ur_platform_handle_t p) {
    uint32_t deviceCount = 0;
    ur_check(urDeviceGetFn(p, UR_DEVICE_TYPE_GPU, 0, nullptr, &deviceCount));

    if (!deviceCount) {
        throw std::runtime_error("No GPUs available.");
    }

    std::vector<ur_device_handle_t> devices(deviceCount);
    ur_check(urDeviceGetFn(p, UR_DEVICE_TYPE_GPU, deviceCount, devices.data(),
                         nullptr));
    return devices;
}

template <typename T, size_t N> struct alignas(PAGE_SIZE) AlignedArray {
    T data[N];
};

int main(int argc, char *argv[]) {
    const char *so_name1 = (argc > 1 && argv[1][0] != '0') ? argv[1] : nullptr;
    printf("LIB1: %s\n", so_name1);
    void *ur_handle = dlopen(so_name1, RTLD_LAZY);
    if (!ur_handle) {
        fprintf(stderr, "%s not found.\n", so_name1);
        exit(1);
    }

    urInitFn = (decltype(urInitFn))dlsym(ur_handle, "urInit");
    urAdapterGetFn =(decltype(urAdapterGetFn)) dlsym(ur_handle, "urAdapterGet");
    urPlatformGetFn = (decltype(urPlatformGetFn))dlsym(ur_handle, "urPlatformGet");
    urDeviceGetFn = (decltype(urDeviceGetFn))dlsym(ur_handle, "urDeviceGet");
    urContextCreateFn = (decltype(urContextCreateFn))dlsym(ur_handle, "urContextCreate");
    urContextGetInfoFn = (decltype(urContextGetInfoFn))dlsym(ur_handle, "urContextGetInfo");
    urAdapterGetInfoFn = (decltype(urAdapterGetInfoFn))dlsym(ur_handle, "urAdapterGetInfo");
    urTearDownFn = (decltype(urTearDownFn))dlsym(ur_handle, "urTearDown");

    if (!urInitFn) {
        fprintf(stderr, "dlsym for ur_handle failed\n");
        exit(1);
    }

    const char *so_name2 = argc == 3 ? argv[2] : nullptr;

    if (!so_name2) {
        printf("LIB2: USING LOADED SYMBOLS\n");
        urInitFn2 = &urInit;
        urContextGetInfoFn2 = &urContextGetInfo;
        urAdapterGetFn2 = &urAdapterGet;
    } else {
        printf("LIB2: %s\n", so_name2);
        void *ur_handle2 = dlopen(so_name2, RTLD_LAZY);
        if (!ur_handle2) {
            fprintf(stderr, "%s not found.\n", so_name2);
            exit(1);
        }

        urInitFn2 = (decltype(urInitFn2))dlsym(ur_handle2, "urInit");
        urContextGetInfoFn2 = (decltype(urContextGetInfoFn2))dlsym(ur_handle2, "urContextGetInfo");
        urAdapterGetFn2 = (decltype(urAdapterGetFn2))dlsym(ur_handle2, "urAdapterGet");

        if (!urInitFn2) {
            fprintf(stderr, "dlsym for ur_handle2 failed\n");
            exit(1);    
        }
    }

    fprintf(stdout, "Handles loaded\n");

    ur_loader_config_handle_t loader_config = nullptr;
    ur_check(urInitFn(UR_DEVICE_INIT_FLAG_GPU, loader_config));

    fprintf(stdout, "Init called\n");

    auto adapters = get_adapters();
    auto supported_adapters = get_supported_adapters(adapters);
    auto platforms = get_platforms(supported_adapters);
    auto gpus = get_gpus(platforms.front());

    constexpr int a_size = 32;
    AlignedArray<int, a_size> a, b;
    for (auto i = 0; i < a_size; ++i) {
        a.data[i] = a_size - i;
        b.data[i] = i;
    }

    auto current_device = gpus.front();

    fprintf(stdout, "Discovery done\n");

    ur_context_handle_t hContext;
    ur_check(urContextCreateFn(1, &current_device, nullptr, &hContext));

    fprintf(stdout, "Context created\n");

    size_t num = 0;
    auto r = urInitFn2(UR_DEVICE_INIT_FLAG_GPU, nullptr);
    if (r != UR_RESULT_SUCCESS) {
        fprintf(stderr, "urInitFn2 failed\n");
        exit(1);
    }

    fprintf(stdout, "Init2 called\n");

    uint32_t cnt = 0;
    r = urAdapterGetFn2(0, nullptr, &cnt);
    if (r != UR_RESULT_SUCCESS) {
        fprintf(stderr, "urAdapterGetFn2 failed\n");
        exit(1);
    }

    r = urContextGetInfoFn2(hContext, UR_CONTEXT_INFO_NUM_DEVICES, sizeof(uint32_t), &num, NULL);
    if (r != UR_RESULT_SUCCESS) {
        fprintf(stderr, "urContextGetInfoFn2 failed\n");
        exit(1);
    }

    printf("Num devices: %lu\n", num);

    return 0;
    //return urTearDown(nullptr) == UR_RESULT_SUCCESS;
}
