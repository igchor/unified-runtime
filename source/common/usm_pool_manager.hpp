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

#include <tuple>
#include <unordered_set>

namespace usm {

struct AllocationDescriptor {
    ur_context_handle_t hContext;
    ur_device_handle_t hDevice;

    size_t size;
    size_t alignment;

    ur_usm_type_t type;
    ur_usm_mem_flags_t flags;
    ur_mem_advice_t advice;
};

struct PerContextDeviceTypePoolManager {
    PerContextDeviceTypePoolManager(): pools(0, &Hash, &Equal) {}

        // template <typename PoolType>
        // PerContextDeviceTypePoolManager(const PoolConfigurations<typename PoolType::Config> &configs, ur_usm_pool_flags_t poolFlags) {
        //     // TODO: should we do it lazily (on first allocation from certain device?)
        //     for (auto &config : configs) {
        //         if (config.first.type == UR_USM_TYPE_HOST) {
        //             assert(!config.first.readOnly); // TODO: should we check this here?
        //             addHostPool(config.second, poolFlags);
        //         } else if (config.first.type == UR_USM_TYPE_DEVICE) {
        //             assert(!config.first.readOnly); // TODO: should we check this here?
        //             addDevicePools(config.second, poolFlags);
        //         } else if (config.first.type == UR_USM_TYPE_SHARED) {
        //             addSharedPools(config.second, config.first.readOnly, poolFlags);
        //         } else {
        //             throw std::runtime_error("Wrong usm type");
        //         }
        //     }
        // }

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

        return poolForDescriptor(AllocationDescriptor{hContext, hDevice, 0, 0, type, flags, ur_mem_advice_t{}});
    }

    void *poolForDescriptor(const AllocationDescriptor& allocDesc) {
        return pools[allocDesc];
    }

    void registerAllocation(const AllocationDescriptor& allocDesc, const void *ptr) {
        if (allocDesc.type == UR_USM_TYPE_SHARED /* && (allocDesc.flags & UR_USM_MEM_FLAG_DEVICE_READ_ONLY) */)
            readOnlyAllocs.insert(ptr);
    }

    void registerFree(const void *ptr) {
        readOnlyAllocs.erase(ptr);
    }

private:
    static constexpr auto consideredMembers = std::make_tuple(
            &AllocationDescriptor::hContext,
            &AllocationDescriptor::hDevice,
            &AllocationDescriptor::type);

    static constexpr bool Equal(const AllocationDescriptor& lhs, const AllocationDescriptor &rhs) {
        auto eq = members_equal(lhs, rhs, consideredMembers);

        //auto lhsReadOnly = lhs.flags & UR_USM_MEM_FLAG_DEVICE_READ_ONLY;
        //auto rhsReadOnly = rhs.flags & UR_USM_MEM_FLAG_DEVICE_READ_ONLY;

        //return eq && (lhsReadOnly == rhsReadOnly);
        return eq;
    };

    static constexpr std::size_t Hash(const AllocationDescriptor& desc) {
        auto h = members_hashed(desc, 0, consideredMembers);
        //return combine_hashes(hash, ((lhs.flags & UR_USM_MEM_FLAG_DEVICE_READ_ONLY) == (rhs.flags & UR_USM_MEM_FLAG_DEVICE_READ_ONLY)));
        return h;
    };

    // template <typename PoolType>
    // void addDevicePools(const typename PoolType::Config &config, ur_usm_pool_flags_t poolFlags) {}
    // template <typename PoolType>
    // void addHostPool(const typename PoolType::Config &config, ur_usm_pool_flags_t poolFlags) {}
    // template <typename PoolType>
    // void addSharedPools(const typename PoolType::Config &config, bool readOnly, ur_usm_pool_flags_t poolFlags) {}

    std::unordered_set<const void*> readOnlyAllocs;
    std::unordered_map<AllocationDescriptor, void*, decltype(&Hash), decltype(&Equal)> pools;
};

// class Allocator {
// public:
//     template <typename PoolType>
//     Allocator(ur_context_handle_t hContext): hContext(hContext), poolManager(hContext, getPoolConfigurationFor<PoolType>("TODO: getenv"), 0) {
//     }

//     template <typename PoolType>
//     std::pair<ur_result_t, void*> createPool(const typename PoolType::Config &config, ur_usm_pool_flags_t poolFlags) {
//         auto pool = new PoolManager(hContext, config, poolFlags);
//         // TODO: error handling
//         return {UR_RESULT_SUCCESS, reinterpret_cast<void*>(pool)};
//     }

//     ur_result_t allocate(const AllocationDescriptor& allocDesc, void *pool = nullptr) { // TODO: make pool part of AllocationDescriptor???
//         auto &poolManager = pool ? *reinterpret_cast<PoolManager*>(pool) : poolManager;
        
//         // TODO: error code translation? for L0 we might want to read tls variable. But here, we can just translate PROVIDER_SPECIFIC TO ADAPTER_SPECIFIC

//         // return umaPoolMalloc(poolManager[strategy.toPool(allocDesc)], allocDesc.size, allocDesc.alignment);
//         // toPool is the place where we decide whether to use read-only or not allocations, etc.
//         // that means that unordered_set<void*> readOnlyAllocs is implemneted inside the strategy.
//     }

// private:
//     ur_context_handle_t hContext;
//     PoolManager poolManager;

//     // PoolingStrategy strategy; TODO: should this just be poolManager? - if so, Allocator should take poolManager as template param
// };

}

#endif /* USM_POOL_MANAGER_HPP */