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

// TODO: cleanup includes

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

#include <umf_helpers.hpp>

struct ur_event_pool_t {
    ur_event_pool_t(ur_context_handle_t Context);

    [[nodiscard]] ur_result_t finalize();

    [[nodiscard]] ur_result_t EventCreate(ur_queue_handle_t Queue,
                        bool IsMultiDevice, bool HostVisible,
                        ur_event_handle_t *RetEvent);

    [[nodiscard]] ur_result_t EventDestroy(ur_event_handle_t Event);

    private:
        // Get index of the free slot in the available pool. If there is no available
        // pool then create new one. The HostVisible parameter tells if we need a
        // slot for a host-visible event. The ProfilingEnabled tells is we need a
        // slot for an event with profiling capabilities.
        ur_result_t getFreeSlotInExistingOrNewPool(ze_event_pool_handle_t &, size_t &,
                                                  bool HostVisible,
                                                  bool ProfilingEnabled,
                                                  ur_device_handle_t Device);

        // Get ur_event_handle_t from cache.
        ur_event_handle_t getEventFromCache(bool HostVisible,
                                                  bool WithProfiling,
                                                  ur_device_handle_t Device);

        // Add ur_event_handle_t to cache.
        void addEventToContextCache(ur_event_handle_t);

        std::list<ze_event_pool_handle_t> *
        getZeEventPoolCache(bool HostVisible, bool WithProfiling,
                            ze_device_handle_t ZeDevice);

        // Decrement number of events living in the pool upon event destroy
        // and return the pool to the cache if there are no unreleased events.
        ur_result_t decrementUnreleasedEventsInPool(ur_event_handle_t Event);

        // Get the cache of events for a provided scope and profiling mode.
        auto getEventCache(bool HostVisible, bool WithProfiling,
                          ur_device_handle_t Device);

    // Context associated with this pool
    ur_context_handle_t Context;

    // Caches for events.
      using EventCache = std::vector<std::list<ur_event_handle_t>>;
      EventCache EventCaches{4};
      std::vector<std::unordered_map<ur_device_handle_t, size_t>>
          EventCachesDeviceMap{4};

    // The cache of event pools from where new events are allocated from.
    // The head event pool is where the next event would be added to if there
    // is still some room there. If there is no room in the head then
    // the following event pool is taken (guranteed to be empty) and made the
    // head. In case there is no next pool, a new pool is created and made the
    // head.
    //
    // Cache of event pools to which host-visible events are added to.
    std::vector<std::list<ze_event_pool_handle_t>> ZeEventPoolCache{4};
    std::vector<std::unordered_map<ze_device_handle_t, size_t>>
        ZeEventPoolCacheDeviceMap{4};

            // This map will be used to determine if a pool is full or not
    // by storing number of empty slots available in the pool.
    std::unordered_map<ze_event_pool_handle_t, uint32_t>
        NumEventsAvailableInEventPool;
    // This map will be used to determine number of unreleased events in the pool.
    // We use separate maps for number of event slots available in the pool from
    // the number of events unreleased in the pool.
    // This will help when we try to make the code thread-safe.
    std::unordered_map<ze_event_pool_handle_t, uint32_t>
        NumEventsUnreleasedInEventPool;

            // Mutex to control operations on event caches.
    ur_mutex EventCacheMutex;

        // Mutex to control operations on event pool caches and the helper maps
    // holding the current pool usage counts.
    ur_mutex ZeEventPoolCacheMutex;
};
