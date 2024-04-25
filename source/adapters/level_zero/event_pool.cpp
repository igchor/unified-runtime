//===--------- context.hpp - Level Zero Adapter ---------------------------===//
//
// Copyright (C) 2023 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "event_pool.hpp"

// Maximum number of events that can be present in an event ZePool is captured
// here. Setting it to 256 gave best possible performance for several
// benchmarks.
static const uint32_t MaxNumEventsPerPool = [] {
  const char *UrRet = std::getenv("UR_L0_MAX_NUMBER_OF_EVENTS_PER_EVENT_POOL");
  const char *PiRet = std::getenv("ZE_MAX_NUMBER_OF_EVENTS_PER_EVENT_POOL");
  const char *MaxNumEventsPerPoolEnv =
      UrRet ? UrRet : (PiRet ? PiRet : nullptr);
  uint32_t Result =
      MaxNumEventsPerPoolEnv ? std::atoi(MaxNumEventsPerPoolEnv) : 256;
  if (Result <= 0)
    Result = 256;
  return Result;
}();

// This is an experimental option that allows to disable caching of events in
// the context.
const bool DisableEventsCaching = [] {
  const char *UrRet = std::getenv("UR_L0_DISABLE_EVENTS_CACHING");
  const char *PiRet = std::getenv("SYCL_PI_LEVEL_ZERO_DISABLE_EVENTS_CACHING");
  const char *DisableEventsCachingFlag =
      UrRet ? UrRet : (PiRet ? PiRet : nullptr);
  if (!DisableEventsCachingFlag)
    return false;
  return std::atoi(DisableEventsCachingFlag) != 0;
}();

ur_event_pool_t::ur_event_pool_t(ur_context_handle_t Context): Context(Context) {}

ur_result_t ur_context_handle_t_::getFreeSlotInExistingOrNewPool(
    ze_event_pool_handle_t &Pool, size_t &Index, bool HostVisible,
    bool ProfilingEnabled, ur_device_handle_t Device) {
  // Lock while updating event pool machinery.
  std::scoped_lock<ur_mutex> Lock(ZeEventPoolCacheMutex);

  ze_device_handle_t ZeDevice = nullptr;

  if (Device) {
    ZeDevice = Device->ZeDevice;
  }
  std::list<ze_event_pool_handle_t> *ZePoolCache =
      getZeEventPoolCache(HostVisible, ProfilingEnabled, ZeDevice);

  if (!ZePoolCache->empty()) {
    if (NumEventsAvailableInEventPool[ZePoolCache->front()] == 0) {
      if (DisableEventsCaching) {
        // Remove full pool from the cache if events caching is disabled.
        ZePoolCache->erase(ZePoolCache->begin());
      } else {
        // If event caching is enabled then we don't destroy events so there is
        // no need to remove pool from the cache and add it back when it has
        // available slots. Just keep it in the tail of the cache so that all
        // pools can be destroyed during context destruction.
        ZePoolCache->push_front(nullptr);
      }
    }
  }
  if (ZePoolCache->empty()) {
    ZePoolCache->push_back(nullptr);
  }

  // We shall be adding an event to the front pool.
  ze_event_pool_handle_t *ZePool = &ZePoolCache->front();
  Index = 0;
  // Create one event ZePool per MaxNumEventsPerPool events
  if (*ZePool == nullptr) {
    ZeStruct<ze_event_pool_desc_t> ZeEventPoolDesc;
    ZeEventPoolDesc.count = MaxNumEventsPerPool;
    ZeEventPoolDesc.flags = 0;
    if (HostVisible)
      ZeEventPoolDesc.flags |= ZE_EVENT_POOL_FLAG_HOST_VISIBLE;
    if (ProfilingEnabled)
      ZeEventPoolDesc.flags |= ZE_EVENT_POOL_FLAG_KERNEL_TIMESTAMP;
    urPrint("ze_event_pool_desc_t flags set to: %d\n", ZeEventPoolDesc.flags);

    std::vector<ze_device_handle_t> ZeDevices;
    if (ZeDevice) {
      ZeDevices.push_back(ZeDevice);
    } else {
      std::for_each(Devices.begin(), Devices.end(),
                    [&](const ur_device_handle_t &D) {
                      ZeDevices.push_back(D->ZeDevice);
                    });
    }

    ZE2UR_CALL(zeEventPoolCreate, (ZeContext, &ZeEventPoolDesc,
                                   ZeDevices.size(), &ZeDevices[0], ZePool));
    NumEventsAvailableInEventPool[*ZePool] = MaxNumEventsPerPool - 1;
    NumEventsUnreleasedInEventPool[*ZePool] = 1;
  } else {
    Index = MaxNumEventsPerPool - NumEventsAvailableInEventPool[*ZePool];
    --NumEventsAvailableInEventPool[*ZePool];
    ++NumEventsUnreleasedInEventPool[*ZePool];
  }
  Pool = *ZePool;
  return UR_RESULT_SUCCESS;
}

ur_event_handle_t ur_context_handle_t_::getEventFromCache(
    bool HostVisible, bool WithProfiling, ur_device_handle_t Device) {
  std::scoped_lock<ur_mutex> Lock(EventCacheMutex);
  auto Cache = getEventCache(HostVisible, WithProfiling, Device);
  if (Cache->empty())
    return nullptr;

  auto It = Cache->begin();
  ur_event_handle_t Event = *It;
  Cache->erase(It);
  // We have to reset event before using it.
  Event->reset();
  return Event;
}

void ur_context_handle_t_::addEventToContextCache(ur_event_handle_t Event) {
  std::scoped_lock<ur_mutex> Lock(EventCacheMutex);
  ur_device_handle_t Device = nullptr;

  if (!Event->IsMultiDevice && Event->UrQueue) {
    Device = Event->UrQueue->Device;
  }

  auto Cache = getEventCache(Event->isHostVisible(),
                             Event->isProfilingEnabled(), Device);
  Cache->emplace_back(Event);
}

ur_result_t
ur_context_handle_t_::decrementUnreleasedEventsInPool(ur_event_handle_t Event) {
  std::shared_lock<ur_shared_mutex> EventLock(Event->Mutex, std::defer_lock);
  std::scoped_lock<ur_mutex, std::shared_lock<ur_shared_mutex>> LockAll(
      ZeEventPoolCacheMutex, EventLock);
  if (!Event->ZeEventPool) {
    // This must be an interop event created on a users's pool.
    // Do nothing.
    return UR_RESULT_SUCCESS;
  }

  ze_device_handle_t ZeDevice = nullptr;

  if (!Event->IsMultiDevice && Event->UrQueue) {
    ZeDevice = Event->UrQueue->Device->ZeDevice;
  }

  std::list<ze_event_pool_handle_t> *ZePoolCache = getZeEventPoolCache(
      Event->isHostVisible(), Event->isProfilingEnabled(), ZeDevice);

  // Put the empty pool to the cache of the pools.
  if (NumEventsUnreleasedInEventPool[Event->ZeEventPool] == 0)
    die("Invalid event release: event pool doesn't have unreleased events");
  if (--NumEventsUnreleasedInEventPool[Event->ZeEventPool] == 0) {
    if (ZePoolCache->front() != Event->ZeEventPool) {
      ZePoolCache->push_back(Event->ZeEventPool);
    }
    NumEventsAvailableInEventPool[Event->ZeEventPool] = MaxNumEventsPerPool;
  }

  return UR_RESULT_SUCCESS;
}

ur_result_t finalize() {
    // TODO: are the mutexes actaully needed?

    if (!DisableEventsCaching) {
    std::scoped_lock<ur_mutex> Lock(EventCacheMutex);
    for (auto &EventCache : EventCaches) {
      for (auto &Event : EventCache) {
        auto ZeResult = ZE_CALL_NOCHECK(zeEventDestroy, (Event->ZeEvent));
        // Gracefully handle the case that L0 was already unloaded.
        if (ZeResult && ZeResult != ZE_RESULT_ERROR_UNINITIALIZED)
          return ze2urResult(ZeResult);
        delete Event;
      }
      EventCache.clear();
    }
  }
  {
    std::scoped_lock<ur_mutex> Lock(ZeEventPoolCacheMutex);
    for (auto &ZePoolCache : ZeEventPoolCache) {
      for (auto &ZePool : ZePoolCache) {
        auto ZeResult = ZE_CALL_NOCHECK(zeEventPoolDestroy, (ZePool));
        // Gracefully handle the case that L0 was already unloaded.
        if (ZeResult && ZeResult != ZE_RESULT_ERROR_UNINITIALIZED)
          return ze2urResult(ZeResult);
      }
      ZePoolCache.clear();
    }
  }
}

// Helper function for creating a UR event.
// The "Queue" argument specifies the UR queue where a command is submitted.
// The "HostVisible" argument specifies if event needs to be allocated from
// a host-visible pool.
//
ur_result_t EventCreate(ur_queue_handle_t Queue,
                        bool IsMultiDevice, bool HostVisible,
                        ur_event_handle_t *RetEvent) {

  bool ProfilingEnabled = !Queue || Queue->isProfilingEnabled();

  ur_device_handle_t Device = nullptr;

  if (!IsMultiDevice && Queue) {
    Device = Queue->Device;
  }

  if (auto CachedEvent = Context->getEventFromCache(
          HostVisible, ProfilingEnabled, Device)) {
    *RetEvent = CachedEvent;
    return UR_RESULT_SUCCESS;
  }

  ze_event_handle_t ZeEvent;
  ze_event_pool_handle_t ZeEventPool = {};

  size_t Index = 0;

  if (auto Res = Context->getFreeSlotInExistingOrNewPool(
          ZeEventPool, Index, HostVisible, ProfilingEnabled, Device))
    return Res;

  ZeStruct<ze_event_desc_t> ZeEventDesc;
  ZeEventDesc.index = Index;
  ZeEventDesc.wait = 0;

  if (HostVisible) {
    ZeEventDesc.signal = ZE_EVENT_SCOPE_FLAG_HOST;
  } else {
    //
    // Set the scope to "device" for every event. This is sufficient for
    // global device access and peer device access. If needed to be seen on
    // the host we are doing special handling, see EventsScope options.
    //
    // TODO: see if "sub-device" (ZE_EVENT_SCOPE_FLAG_SUBDEVICE) can better be
    //       used in some circumstances.
    //
    ZeEventDesc.signal = 0;
  }

  ZE2UR_CALL(zeEventCreate, (ZeEventPool, &ZeEventDesc, &ZeEvent));

  try {
    *RetEvent = new ur_event_handle_t_(
        ZeEvent, ZeEventPool, reinterpret_cast<ur_context_handle_t>(Context),
        UR_EXT_COMMAND_TYPE_USER, true);
  } catch (const std::bad_alloc &) {
    return UR_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  } catch (...) {
    return UR_RESULT_ERROR_UNKNOWN;
  }

  if (HostVisible)
    (*RetEvent)->HostVisibleEvent =
        reinterpret_cast<ur_event_handle_t>(*RetEvent);

  return UR_RESULT_SUCCESS;
}

std::list<ze_event_pool_handle_t> *
        getZeEventPoolCache(bool HostVisible, bool WithProfiling,
                            ze_device_handle_t ZeDevice) {
          if (HostVisible) {
            if (ZeDevice) {
              auto ZeEventPoolCacheMap = WithProfiling
                                            ? &ZeEventPoolCacheDeviceMap[0]
                                            : &ZeEventPoolCacheDeviceMap[1];
              if (ZeEventPoolCacheMap->find(ZeDevice) == ZeEventPoolCacheMap->end()) {
                ZeEventPoolCache.emplace_back();
                ZeEventPoolCacheMap->insert(
                    std::make_pair(ZeDevice, ZeEventPoolCache.size() - 1));
              }
              return &ZeEventPoolCache[(*ZeEventPoolCacheMap)[ZeDevice]];
            } else {
              return WithProfiling ? &ZeEventPoolCache[0] : &ZeEventPoolCache[1];
            }
          } else {
            if (ZeDevice) {
              auto ZeEventPoolCacheMap = WithProfiling
                                            ? &ZeEventPoolCacheDeviceMap[2]
                                            : &ZeEventPoolCacheDeviceMap[3];
              if (ZeEventPoolCacheMap->find(ZeDevice) == ZeEventPoolCacheMap->end()) {
                ZeEventPoolCache.emplace_back();
                ZeEventPoolCacheMap->insert(
                    std::make_pair(ZeDevice, ZeEventPoolCache.size() - 1));
              }
              return &ZeEventPoolCache[(*ZeEventPoolCacheMap)[ZeDevice]];
            } else {
              return WithProfiling ? &ZeEventPoolCache[2] : &ZeEventPoolCache[3];
            }
          }
        }

auto getEventCache(bool HostVisible, bool WithProfiling,
                          ur_device_handle_t Device) {
          if (HostVisible) {
            if (Device) {
              auto EventCachesMap =
                  WithProfiling ? &EventCachesDeviceMap[0] : &EventCachesDeviceMap[1];
              if (EventCachesMap->find(Device) == EventCachesMap->end()) {
                EventCaches.emplace_back();
                EventCachesMap->insert(
                    std::make_pair(Device, EventCaches.size() - 1));
              }
              return &EventCaches[(*EventCachesMap)[Device]];
            } else {
              return WithProfiling ? &EventCaches[0] : &EventCaches[1];
            }
          } else {
            if (Device) {
              auto EventCachesMap =
                  WithProfiling ? &EventCachesDeviceMap[2] : &EventCachesDeviceMap[3];
              if (EventCachesMap->find(Device) == EventCachesMap->end()) {
                EventCaches.emplace_back();
                EventCachesMap->insert(
                    std::make_pair(Device, EventCaches.size() - 1));
              }
              return &EventCaches[(*EventCachesMap)[Device]];
            } else {
              return WithProfiling ? &EventCaches[2] : &EventCaches[3];
            }
          }
        }

ur_result_t EventDestroy(ur_event_handle_t Event) {
    if (!DisableEventsCaching) {
        return UR_RESULT_SUCCESS;
    }

      auto ZeResult = ZE_CALL_NOCHECK(zeEventDestroy, (Event->ZeEvent));
      // Gracefully handle the case that L0 was already unloaded.
      if (ZeResult && ZeResult != ZE_RESULT_ERROR_UNINITIALIZED)
        return ze2urResult(ZeResult);
      auto Context = Event->Context;
      if (auto Res = Context->decrementUnreleasedEventsInPool(Event))
        return Res;
    
  // It is possible that host-visible event was never created.
  // In case it was check if that's different from this same event
  // and release a reference to it.
  if (Event->HostVisibleEvent && Event->HostVisibleEvent != Event) {
    // Decrement ref-count of the host-visible proxy event.
    UR_CALL(urEventReleaseInternal(Event->HostVisibleEvent));
  }
  
  if (DisableEventsCaching || !Event->OwnNativeHandle) {
    delete Event;
  } else {
    Event->Context->EventPool.add(Event);
  }
}
