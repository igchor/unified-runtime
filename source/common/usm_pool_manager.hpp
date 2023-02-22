/*
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef USM_POOL_MANAGER_HPP
#define USM_POOL_MANAGER_HPP 1

#include <optional>

namespace usm {

struct AllocationDescriptor {
    AllocationDescriptor(ur_context_handle_t hContext, ur_device_handle_t hDevice, ur_usm_type_t type, ur_usm_mem_flags_t flags, ur_mem_advice_t advice)
        : hContext(hContext), hDevice(hDevice), type(type), flags(flags), advice(advice)  {
        assert(hContext);
        assert(type == UR_USM_TYPE_HOST || hDevice);
        assert(type != UR_USM_TYPE_UNKOWN);
    }

    bool operator==(const AllocationDescriptor &other) const noexcept {
        return type == other.type && readOnly == other.readOnly && hDevice == other.hDevice && hContext == other.hContext;
    }

    ur_context_handle_t hContext;
    ur_device_handle_t hDevice;

    ur_usm_type_t type;
    ur_usm_mem_flags_t flags;
    ur_mem_advice_t advice;

    size_t size;
    size_t alignment;
};

struct PoolingStrategy {
    // some tracking function? (trackAllocation, trackFree?)
    XXX toPoolDesc(const AllocationDescriptor& allocDesc) = 0; // TODO: what should we return?
};

}

template <>
struct std::hash<usm::AllocationDescriptor> {
    std::size_t operator()(const usm::AllocationDescriptor &p) const noexcept {
        return combine_hashes(0, p.hDevice, p.hContext, p.type, p.flags, p.advice);
    }
};

namespace usm {

// usm_mem_desc_t.pool = PoolManager*

// TODO: define interface if this should also contain strategy for selecting pooling granularity
class PoolManager {
public:
    struct Descriptor { // THIS structure defines pooling granularity
        ur_context_handle_t hContext;
        ur_device_handle_t hDevice;

        ur_usm_type_t type;
        ur_usm_mem_flags_t flags;
        ur_mem_advice_t advice;
    };

    // this function decides how to group allocations, can ignore certain filed from desc
    // Descriptor AllocationDescriptorToPoolDescriptor(const AllocationDescriptor& desc);
    // void addPools(const PoolConfigurations<typename PoolType::Config> &configs); -> std::unorder_map<Descriptor, uma_memory_pool_handle_t>;

    template <typename PoolType>
    PoolManager(ur_context_handle_t hContext, const PoolConfigurations<typename PoolType::Config> &configs, ur_usm_pool_flags_t poolFlags): hContext(hContext) {
        // TODO: should we do it lazily (on first allocation from certain device?)
        for (auto &config : configs) {
            if (config.first.type == UR_USM_TYPE_HOST) {
                assert(!config.first.readOnly); // TODO: should we check this here?
                addPoolForHost(config.second, poolFlags);
            } else if (config.first.type == UR_USM_TYPE_DEVICE) {
                assert(!config.first.readOnly); // TODO: should we check this here?
                addPoolForDevices(config.second, poolFlags);
            } else if (config.first.type == UR_USM_TYPE_SHARED) {
                addPoolForShared(config.second, config.first.readOnly, poolFlags);
            } else {
                throw std::runtime_error("Wrong usm type");
            }
        }
    }

    uma_memory_pool_handle_t getPool(const void *ptr) {
        ur_usm_type_t type;
        auto ret = urUSMGetMemAllocInfo(hContext, ptr, UR_USM_ALLOC_INFO_TYPE, sizeof(ur_usm_type_t), &type, nullptr);
        if (ret != UR_RESULT_SUCCESS) {
            throw std::runtime_error("TODO");
        }

        ur_device_handle_t hDevice = nullptr;
        if (type != UR_USM_TYPE_HOST) {
            ret = urUSMGetMemAllocInfo(hContext, ptr, UR_USM_ALLOC_INFO_DEVICE, sizeof(pi_hDevice), &hDevice, nullptr);
            if (ret != UR_RESULT_SUCCESS) {
                throw std::runtime_error("TODO");
            }
        }

        bool readOnly = false;
        if (type == UR_USM_TYPE_SHARED && readOnlyAllocs.count(ptr) > 0) {
            readOnly = true;
        }

        return AllocationDescriptor(hContext, hDevice, type, readOnly);
    }

    // TODO: can we separate tracking from pool management through strategy?
    void trackAllocation(const AllocationDescriptor& allocDesc, const void *ptr) {
        if (allocDesc.type == UR_USM_TYPE_SHARED && allocDesc.readOnly)
            readOnlyAllocs.insert(ptr);
    }

    void trackFree(const void *ptr) {
        readOnlyAllocs.erase(ptr);
    }

    std::unordered_set<const void*> readOnlyAllocs; // TODO: should this be implemented on UMA level?
    std::unordered_map<Descriptor, uma_memory_pool_handle_t> pools;

    template <typename PoolType>
    void addPoolForDevices(const typename PoolType::Config &config, ur_usm_pool_flags_t poolFlags);
    template <typename PoolType>
    void addPoolForHost(const typename PoolType::Config &config, ur_usm_pool_flags_t poolFlags);
    template <typename PoolType>
    void addPoolForShared(const typename PoolType::Config &config, bool readOnly, ur_usm_pool_flags_t poolFlags);
};

class Allocator {
public:
    template <typename PoolType>
    Allocator(ur_context_handle_t hContext): hContext(hContext), poolManager(hContext, getPoolConfigurationFor<PoolType>("TODO: getenv"), 0) {
    }

    template <typename PoolType>
    std::pair<ur_result_t, void*> createPool(const typename PoolType::Config &config, ur_usm_pool_flags_t poolFlags) {
        auto pool = new PoolManager(hContext, config, poolFlags);
        // TODO: error handling
        return {UR_RESULT_SUCCESS, reinterpret_cast<void*>(pool)};
    }

    ur_result_t allocate(const AllocationDescriptor& allocDesc, void *pool = nullptr) { // TODO: make pool part of AllocationDescriptor???
        auto &poolManager = pool ? *reinterpret_cast<PoolManager*>(pool) : poolManager;
        
        // TODO: error code translation? for L0 we might want to read tls variable. But here, we can just translate PROVIDER_SPECIFIC TO ADAPTER_SPECIFIC

        // return umaPoolMalloc(poolManager[strategy.toPool(allocDesc)], allocDesc.size, allocDesc.alignment);
        // toPool is the place where we decide whether to use read-only or not allocations, etc.
        // that means that unordered_set<void*> readOnlyAllocs is implemneted inside the strategy.
    }

private:
    ur_context_handle_t hContext;
    PoolManager poolManager;

    // PoolingStrategy strategy; TODO: should this just be poolManager? - if so, Allocator should take poolManager as template param
};

}

#endif /* USM_POOL_MANAGER_HPP */