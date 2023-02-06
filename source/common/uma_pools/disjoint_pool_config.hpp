//===--- disjoint_pool_config.hpp -configuration for USM memory pool--------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef USM_POOL_CONFIG
#define USM_POOL_CONFIG

#include "disjoint_pool.hpp"
#include "ur_api.h"
#include "ur_util.hpp"

#include <string>
#include <unordered_map>

namespace usm {
struct PoolDescriptor {
    PoolDescriptor(ur_usm_type_t type, bool readOnly = false)
        : type(type), readOnly(readOnly) {}

    bool operator==(const PoolDescriptor &other) const noexcept {
        return type == other.type && readOnly == other.readOnly;
    }

    ur_usm_type_t type;
    bool readOnly;
};
} // namespace usm

template <> struct std::hash<usm::PoolDescriptor> {
    std::size_t operator()(const usm::PoolDescriptor &p) const noexcept {
        return combine_hashes(0, p.type, p.readOnly);
    }
};

namespace usm {
template <typename Config>
using PoolConfigurations = std::unordered_map<PoolDescriptor, Config>;

// Parses config string into <PoolDescriptor, PoolType::Config> map
// Parse optional parameters of this form:
// [EnableBuffers][;[MaxPoolSize][;memtypelimits]...]
//  memtypelimits: [<memtype>:]<limits>
//  memtype: host|device|shared
//  limits:  [MaxPoolableSize][,[Capacity][,SlabMinSize]]
//
// Without a memory type, the limits are applied to each memory type.
// Parameters are for each context, except MaxPoolSize, which is overall
// pool size for all contexts.
// Duplicate specifications will result in the right-most taking effect.
//
// EnableBuffers:   Apply chunking/pooling to SYCL buffers.
//                  Default 1.
// MaxPoolSize:     Limit on overall unfreed memory.
//                  Default 16MB.
// MaxPoolableSize: Maximum allocation size subject to chunking/pooling.
//                  Default 2MB host, 4MB device and 0 shared.
// Capacity:        Maximum number of unfreed allocations in each bucket.
//                  Default 4.
// SlabMinSize:     Minimum allocation size requested from USM.
//                  Default 64KB host and device, 2MB shared.
//
// Example of usage:
// "1;32M;host:1M,4,64K;device:1M,4,64K;shared:0,0,2M"
PoolConfigurations<typename DisjointPool::Config>
getPoolConfigurations(const std::string &config, bool trace);

} // namespace usm