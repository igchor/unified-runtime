//===--------- context.hpp - Level Zero Adapter ---------------------------===//
//
// Copyright (C) 2023 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#pragma once

#include <list>
#include <map>
#include <stdarg.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <ur/ur.hpp>
#include <ur_api.h>
#include <ze_api.h>
#include <zes_api.h>

#include "common.hpp"
#include "queue.hpp"
#include "event_pool.hpp"

#include <umf_helpers.hpp>

struct l0_command_list_cache_info {
  ZeStruct<ze_command_queue_desc_t> ZeQueueDesc;
  bool InOrderList = false;
};

struct ur_context_handle_t_ : _ur_object {
  ur_context_handle_t_(ze_context_handle_t ZeContext, uint32_t NumDevices,
                       const ur_device_handle_t *Devs, bool OwnZeContext)
      : ZeContext{ZeContext}, Devices{Devs, Devs + NumDevices},
        NumDevices{NumDevices} {
    OwnNativeHandle = OwnZeContext;
  }

  ur_context_handle_t_(ze_context_handle_t ZeContext) : ZeContext{ZeContext} {}

  // A L0 context handle is primarily used during creation and management of
  // resources that may be used by multiple devices.
  // This field is only set at ur_context_handle_t creation time, and cannot
  // change. Therefore it can be accessed without holding a lock on this
  // ur_context_handle_t.
  const ze_context_handle_t ZeContext{};

  // Keep the PI devices this PI context was created for.
  // This field is only set at ur_context_handle_t creation time, and cannot
  // change. Therefore it can be accessed without holding a lock on this
  // ur_context_handle_t. const std::vector<ur_device_handle_t> Devices;
  std::vector<ur_device_handle_t> Devices;
  uint32_t NumDevices{};

  // Immediate Level Zero command list for the device in this context, to be
  // used for initializations. To be created as:
  // - Immediate command list: So any command appended to it is immediately
  //   offloaded to the device.
  // - Synchronous: So implicit synchronization is made inside the level-zero
  //   driver.
  // There will be a list of immediate command lists (for each device) when
  // support of the multiple devices per context will be added.
  ze_command_list_handle_t ZeCommandListInit{};

  // Mutex for the immediate command list. Per the Level Zero spec memory copy
  // operations submitted to an immediate command list are not allowed to be
  // called from simultaneous threads.
  ur_mutex ImmediateCommandListMutex;

  // Mutex Lock for the Command List Cache. This lock is used to control both
  // compute and copy command list caches.
  ur_mutex ZeCommandListCacheMutex;

  // If context contains one device or sub-devices of the same device, we want
  // to save this device.
  // This field is only set at ur_context_handle_t creation time, and cannot
  // change. Therefore it can be accessed without holding a lock on this
  // ur_context_handle_t.
  ur_device_handle_t SingleRootDevice = nullptr;

  // Cache of all currently available/completed command/copy lists.
  // Note that command-list can only be re-used on the same device.
  //
  // TODO: explore if we should use root-device for creating command-lists
  // as spec says that in that case any sub-device can re-use it: "The
  // application must only use the command list for the device, or its
  // sub-devices, which was provided during creation."
  //
  std::unordered_map<ze_device_handle_t,
                     std::list<std::pair<ze_command_list_handle_t,
                                         l0_command_list_cache_info>>>
      ZeComputeCommandListCache;
  std::unordered_map<ze_device_handle_t,
                     std::list<std::pair<ze_command_list_handle_t,
                                         l0_command_list_cache_info>>>
      ZeCopyCommandListCache;

  // Store USM pool for USM shared and device allocations. There is 1 memory
  // pool per each pair of (context, device) per each memory type.
  std::unordered_map<ze_device_handle_t, umf::pool_unique_handle_t>
      DeviceMemPools;
  std::unordered_map<ze_device_handle_t, umf::pool_unique_handle_t>
      SharedMemPools;
  std::unordered_map<ze_device_handle_t, umf::pool_unique_handle_t>
      SharedReadOnlyMemPools;

  // Store the host memory pool. It does not depend on any device.
  umf::pool_unique_handle_t HostMemPool;

  // Allocation-tracking proxy pools for direct allocations. No pooling used.
  std::unordered_map<ze_device_handle_t, umf::pool_unique_handle_t>
      DeviceMemProxyPools;
  std::unordered_map<ze_device_handle_t, umf::pool_unique_handle_t>
      SharedMemProxyPools;
  std::unordered_map<ze_device_handle_t, umf::pool_unique_handle_t>
      SharedReadOnlyMemProxyPools;
  umf::pool_unique_handle_t HostMemProxyPool;

  // Map associating pools created with urUsmPoolCreate and internal pools
  std::list<ur_usm_pool_handle_t> UsmPoolHandles{};

  // We need to store all memory allocations in the context because there could
  // be kernels with indirect access. Kernels with indirect access start to
  // reference all existing memory allocations at the time when they are
  // submitted to the device. Referenced memory allocations can be released only
  // when kernel has finished execution.
  std::unordered_map<void *, MemAllocRecord> MemAllocs;

  // Handles event creation and caching
  ur_event_pool_t EventPool;

  // Initialize the PI context.
  ur_result_t initialize();

  // If context contains one device then return this device.
  // If context contains sub-devices of the same device, then return this parent
  // device. Return nullptr if context consists of several devices which are not
  // sub-devices of the same device. We call returned device the root device of
  // a context.
  // TODO: get rid of this when contexts with multiple devices are supported for
  // images.
  ur_device_handle_t getRootDevice() const;

  // Finalize the PI context
  ur_result_t finalize();

  // Return the Platform, which is the same for all devices in the context
  ur_platform_handle_t getPlatform() const;

  // Retrieves a command list for executing on this device along with
  // a fence to be used in tracking the execution of this command list.
  // If a command list has been created on this device which has
  // completed its commands, then that command list and its associated fence
  // will be reused. Otherwise, a new command list and fence will be created for
  // running on this device. L0 fences are created on a L0 command queue so the
  // caller must pass a command queue to create a new fence for the new command
  // list if a command list/fence pair is not available. All Command Lists &
  // associated fences are destroyed at Device Release.
  // If UseCopyEngine is true, the command will eventually be executed in a
  // copy engine. Otherwise, the command will be executed in a compute engine.
  // If AllowBatching is true, then the command list returned may already have
  // command in it, if AllowBatching is false, any open command lists that
  // already exist in Queue will be closed and executed.
  // If ForcedCmdQueue is not nullptr, the resulting command list must be tied
  // to the contained command queue. This option is ignored if immediate
  // command lists are used.
  // When using immediate commandlists, retrieves an immediate command list
  // for executing on this device. Immediate commandlists are created only
  // once for each SYCL Queue and after that they are reused.
  ur_result_t
  getAvailableCommandList(ur_queue_handle_t Queue,
                          ur_command_list_ptr_t &CommandList,
                          bool UseCopyEngine, bool AllowBatching = false,
                          ze_command_queue_handle_t *ForcedCmdQueue = nullptr);

  // Checks if Device is covered by this context.
  // For that the Device or its root devices need to be in the context.
  bool isValidDevice(ur_device_handle_t Device) const;
};

// Helper function to release the context, a caller must lock the platform-level
// mutex guarding the container with contexts because the context can be removed
// from the list of tracked contexts.
ur_result_t ContextReleaseHelper(ur_context_handle_t Context);
