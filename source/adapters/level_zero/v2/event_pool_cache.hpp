//===--------- event_pool_cache.hpp - Level Zero Adapter ------------------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <stack>

#include <unordered_map>
#include <ur/ur.hpp>
#include <ur_api.h>
#include <ze_api.h>

#include "../device.hpp"
#include "event_pool.hpp"
#include "event_provider.hpp"

namespace v2 {

namespace raii {
using cache_borrowed_event_pool =
    std::unique_ptr<event_pool, std::function<void(event_pool *)>>;
} // namespace raii

class event_pool_cache {
public:
  using ProviderCreateFunc = std::function<std::unique_ptr<event_provider>(
      DeviceId, bool profilingEnabled)>;

  event_pool_cache(size_t max_devices, ProviderCreateFunc);
  ~event_pool_cache();

  raii::cache_borrowed_event_pool borrow(DeviceId, bool profilingEnabled);

private:
  ur_mutex mutex;
  ProviderCreateFunc providerCreate;

  using per_profiling_events_vec = std::vector<std::unique_ptr<event_pool>>;
  using per_device_events_vec = std::vector<per_profiling_events_vec>;
  using pools_vec = std::vector<per_device_events_vec>;

  pools_vec pools;
};

} // namespace v2
