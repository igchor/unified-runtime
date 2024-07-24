//===--------- command_list_cache.hpp - Level Zero Adapter ---------------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#pragma once

#include <memory>
#include <mutex>
#include <stack>

#include <unordered_map>
#include <ur/ur.hpp>
#include <ur_api.h>
#include <vector>
#include <ze_api.h>

namespace v2 {

enum event_type { EVENT_REGULAR, EVENT_COUNTER };

class event_deleter;

namespace raii {
// custom deleter to avoid memory allocations
struct event_cache_deleter {
  inline event_cache_deleter() : deleter(nullptr) {}
  inline event_cache_deleter(event_deleter *deleter) : deleter(deleter) {}
  inline void operator()(::ze_event_handle_t event) const;

private:
  event_deleter *deleter;
};

using cache_borrowed_event =
    std::unique_ptr<_ze_event_handle_t, event_cache_deleter>;
} // namespace raii

struct event_allocation {
  event_type type;
  raii::cache_borrowed_event borrow;
};

class event_deleter {
public:
  virtual void free(::ze_event_handle_t event) = 0;
};

class event_provider {
public:
  virtual ~event_provider() = default;
  virtual event_allocation allocate() = 0;
  virtual ur_device_handle_t device() = 0;
};

inline void
raii::event_cache_deleter::operator()(::ze_event_handle_t event) const {
  if (!deleter)
    return;

  deleter->free(event);
}

} // namespace v2
