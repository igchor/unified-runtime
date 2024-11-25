//===--------- event_pool.cpp - Level Zero Adapter ------------------------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "event_pool.hpp"
#include "common/latency_tracker.hpp"
#include "event.hpp"
#include "queue_api.hpp"
#include "ur_api.h"

namespace v2 {

static constexpr size_t EVENTS_BURST = 64;

static uint64_t CHECK_EXECUTING = []() {
  return getenv_to_unsigned("UR_L0_V2_CHECK_EXECUTING").value_or(16);
}();

static uint64_t EVENT_POOL_CLEANUP_SIZE = []() {
  return getenv_to_unsigned("UR_L0_V2_EVENT_POOL_CLEANUP_SIZE").value_or(1000);
}();

ur_pooled_event_t *event_pool::allocate() {
  TRACK_SCOPE_LATENCY("event_pool::allocate");

  std::unique_lock<std::mutex> lock(*mutex);

  if (freelist.empty()) {
    auto start = events.size();
    auto end = start + EVENTS_BURST;
    for (; start < end; ++start) {
      events.emplace_back(hContext, provider->allocate(), this);
      freelist.push_back(&events.at(start));
    }
  }

  auto event = freelist.back();
  freelist.pop_back();

  event->resetQueueAndCommand(nullptr, UR_COMMAND_FORCE_UINT32);

  return event;
}

void event_pool::forceCleanupExecuting() {
  TRACK_SCOPE_LATENCY("event_pool::forceCleanupExecuting");

  std::unique_lock<std::mutex> lock(*mutex);

  for (auto &event : executing) {
    event->reset();
    freelist.push_back(event);
  }

  executing.clear();
}

void event_pool::cleanupExecuting() {
  TRACK_SCOPE_LATENCY("event_pool::cleanupExecuting");

  std::unique_lock<std::mutex> lock(*mutex);

  if (executing.size() < EVENT_POOL_CLEANUP_SIZE) {
    return;
  }

  size_t completed = 0;
  for (size_t i = 0; i < std::min(CHECK_EXECUTING, executing.size()); i++) {
    if (zeEventQueryStatus(executing[i]->getZeEvent()) == ZE_RESULT_SUCCESS) {
      executing[i]->reset();
      freelist.push_back(executing[i]);
      completed++;
    } else {
      break;
    }
  }

  executing.erase(executing.begin(), executing.begin() + completed);
}

void event_pool::free(ur_pooled_event_t *event, bool completed) {
  TRACK_SCOPE_LATENCY("event_pool::free");

  std::unique_lock<std::mutex> lock(*mutex);

  if (completed) {
    event->reset();
    freelist.push_back(event);
  } else {
    executing.push_back(event);
  }

  // The event is still in the pool, so we need to increment the refcount
  assert(event->RefCount.load() == 0);
  event->RefCount.increment();
}

event_provider *event_pool::getProvider() const { return provider.get(); }

event_flags_t event_pool::getFlags() const {
  return getProvider()->eventFlags();
}

} // namespace v2
