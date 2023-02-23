/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef USM_POOL_MANAGER_HPP
#define USM_POOL_MANAGER_HPP 1

#include "ur_util.hpp"
#include "ur_api.h"

#include <uma/memory_pool.h>

#include <tuple>
#include <unordered_set>
#include <shared_mutex>

namespace usm {

// TODO
struct ur_usm_pool_flags_t{};
template <typename Config>
using PoolConfigurations = std::unordered_map<void*, Config>;

struct AllocationDescriptor {
    ur_context_handle_t hContext;
    ur_device_handle_t hDevice;

    size_t size;
    size_t alignment;

    ur_usm_type_t type;
    ur_usm_mem_flags_t flags;
    ur_mem_advice_t advice;

    void *pool;
};

struct PerContextDeviceTypePoolManager {
    static constexpr size_t PoolsPerDevice = 4; /* Host, Device, Shared, SharedReadOnly */

    template <typename PoolType>
    PerContextDeviceTypePoolManager(ur_context_handle_t hContext, uint32_t DeviceCount,
        const ur_device_handle_t *phDevices, const PoolConfigurations<typename PoolType::Config> &configs, ur_usm_pool_flags_t poolFlags):
        hContext(hContext), DeviceCount(DeviceCount), phDevices(phDevices), pools(devices.size() * PoolsPerDevice, &Hash, &Equal) {
        for (auto &config : configs) {
            addPools(urManagedPools, config, poolFlags);
        }
    }

    template <typename PoolType>
    void* poolCreate(const typename PoolType::Config &config, ur_usm_pool_flags_t poolFlags) {
        auto pools = new PerDeviceTypeMap;
        addPools(pools, config, poolFlags);
        return reinterpret_cast<void*>(pools);
    }

    void poolDestroy(void *pool) {
        delete *reinterpret_cast<PerDeviceTypeMap*>(pool);
    }

    void* poolForPtr(ur_context_handle_t hContext, const void *ptr) {
        ur_usm_type_t type;
        auto ret = urUSMGetMemAllocInfo(hContext, ptr, UR_USM_ALLOC_INFO_TYPE, sizeof(ur_usm_type_t), &type, nullptr);
        if (ret != UR_RESULT_SUCCESS) {
            // throw std::runtime_error("TODO");
        }

        ur_device_handle_t hDevice = nullptr;
        if (type != UR_USM_TYPE_HOST) {
            ret = urUSMGetMemAllocInfo(hContext, ptr, UR_USM_ALLOC_INFO_DEVICE, sizeof(ur_device_handle_t), &hDevice, nullptr);
            if (ret != UR_RESULT_SUCCESS) {
                // throw std::runtime_error("TODO");
            }
        }

        ur_usm_mem_flags_t flags = 0;
        if (type == UR_USM_TYPE_SHARED && readOnlyAllocs.count(ptr) > 0) {
            // flags |= UR_USM_MEM_FLAG_DEVICE_READ_ONLY;
        }

        return poolForDescriptor({hContext, hDevice, 0, 0, type, flags, ur_mem_advice_t{}, nullptr});
    }

    void *poolForDescriptor(const AllocationDescriptor& allocDesc) {
        auto &pools = allocDesc.pool ? *reinterpret_cast<PerDeviceTypeMap*>(allocDesc.pool) : urManagedPools;

        auto poolIt = pools.find(allocDesc);
        if (poolIt == pools.end()) {
            // LOG
            return nullptr;
        }

        return poolIt->second;
    }

    void registerAllocation(const AllocationDescriptor& allocDesc, const void *ptr) {
        // TODO: lock?
        if (isSharedAllocationReadOnlyOnDevice(allocDesc)) {
            readOnlyAllocs.insert(ptr);
        }
    }

    void registerFree(const void *ptr) {
        // TODO: lock?
        readOnlyAllocs.erase(ptr);
    }

private:
    static constexpr auto members = std::make_tuple(
            &AllocationDescriptor::hDevice,
            &AllocationDescriptor::type);

    static constexpr bool isSharedAllocationReadOnlyOnDevice(const AllocationDescriptor& desc) {
        return false;
        // return (desc.type == UR_USM_TYPE_SHARED) && (desc & UR_USM_MEM_FLAG_DEVICE_READ_ONLY);
    }

    static constexpr bool Equal(const AllocationDescriptor& lhs, const AllocationDescriptor &rhs) {
        auto eq = members_equal(lhs, rhs, members);
        //return eq && (isSharedAllocationReadOnlyOnDevice(lhs) == isSharedAllocationReadOnlyOnDevice(rhs));
        return eq;
    };

    static constexpr std::size_t Hash(const AllocationDescriptor& desc) {
        auto h = members_hashed(desc, 0, members);
        //return combine_hashes(hash, (isSharedAllocationReadOnlyOnDevice(lhs) == isSharedAllocationReadOnlyOnDevice(rhs)));
        return h;
    };

    using PerDeviceTypeMap = std::unordered_map<AllocationDescriptor, void*, decltype(&Hash), decltype(&Equal)>;

    template <typename PoolType>
    void addPools(PerDeviceTypeMap& poolMap, const typename PoolType::Config &config, ur_usm_pool_flags_t poolFlags) {
        addHostPool(poolMap, config, poolFlags);
        addDevicePools(poolMap, config, poolFlags);
        addSharedPools(poolMap, config, poolFlags);
    }

    template <typename PoolType>
    void addDevicePools(PerDeviceTypeMap& poolMap, const typename PoolType::Config &config, ur_usm_pool_flags_t poolFlags, size_t poolId) {

    }
    template <typename PoolType>
    void addHostPool(PerDeviceTypeMap& poolMap, const typename PoolType::Config &config, ur_usm_pool_flags_t poolFlags, size_t poolId) {

    }
    template <typename PoolType>
    void addSharedPools(PerDeviceTypeMap& poolMap, const typename PoolType::Config &config, ur_usm_pool_flags_t poolFlags, size_t poolId) {

    }

    ur_context_handle_t hContext;
    uint32_t DeviceCount;
    const ur_device_handle_t *phDevices;

    std::shared_mutex mtx;
    std::unordered_set<const void*> readOnlyAllocs;

    PerDeviceTypeMap urManagedPools;
};

template <typename PoolManager>
class Allocator {
public:
    template <typename PoolType>
    Allocator(ur_context_handle_t hContext, uint32_t DeviceCount,
        const ur_device_handle_t *phDevices): hContext(hContext), poolManager(hContext, DeviceCount, phDevices, 0) {
    }

    template <typename PoolType>
    std::pair<ur_result_t, void*> poolCreate(const typename PoolType::Config &config, ur_usm_pool_flags_t poolFlags) {
        try {
            auto pool = poolManager.poolCreate(config, poolFlags);
            return {UR_RESULT_SUCCESS, pool};
        } catch (std::bad_alloc&) {
            return {UR_RESULT_ERROR_OUT_OF_HOST_MEMORY, nullptr};
        } catch (...) {
            return {UR_RESULT_ERROR_UNKNOWN, nullptr};
        }
    }
    
    ur_result_t poolDestroy(void* pool) {
        poolManager.poolDestroy(pool);
    }

    ur_result_t allocate(const AllocationDescriptor& allocDesc) {
        // TODO: we could do something like this, but how about free?
        // auto &poolManager = pool ? *reinterpret_cast<PoolManager*>(pool) : poolManager;

        // TODO: error code translation

        auto pool = poolManager.poolForDescriptor(allocDesc); // TODO: error handling
        auto ptr = umaPoolMalloc(ret, allocDesc.size, allocDesc.alignment);
        if (ptr) {
            poolManager.trackAllocation(ptr);
        }

        return ptr;
    }

    void free(const void *ptr) {
        auto pool = poolManager.poolForPtr(ptr);
        umaPoolFree(pool, ptr);
        poolManager.trackFree(ptr);
    }

private:
    ur_context_handle_t hContext;
    PoolManager poolManager;
};

}

#endif /* USM_POOL_MANAGER_HPP */