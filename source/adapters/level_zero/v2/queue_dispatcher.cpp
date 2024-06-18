//===--------- queue_dispatcher.cpp - Level Zero Adapter -----------------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <cassert>

#include "queue_dispatcher.hpp"

namespace v2 {

  ur_queue_dispatcher_t::ur_queue_dispatcher_t(ur_context_handle_t Context, ur_device_handle_t Device,
                        ur_queue_flags_t Flags)
      : Queue(initialize(Context, Device, Flags)) {}

  void ur_queue_dispatcher_t::release() {
    if (this->RefCount.decrementAndTest()) {
      delete this;
    }
  }

  void ur_queue_dispatcher_t::retain() {
    this->RefCount.increment();
  }


  std::variant<ur_queue_immediate_in_order_t>
  ur_queue_dispatcher_t::initialize(ur_context_handle_t Context, ur_device_handle_t Device,
             ur_queue_flags_t Flags) {
    return ur_queue_immediate_in_order_t(Context, Device, Flags);
  }

bool shouldUseDispatcher(ur_queue_flags_t Flags, ur_device_handle_t Device) {
  const char *UrRet = std::getenv("UR_L0_USE_QUEUE_DISPATCHER");
  bool UseDispatcher = UrRet && std::stoi(UrRet);

  // Right now, only immediate, in-order queues are supported
  return UseDispatcher && (Flags & UR_QUEUE_FLAG_SUBMISSION_IMMEDIATE) &&
         Device->useImmediateCommandLists() &&
         (Flags & UR_QUEUE_FLAG_OUT_OF_ORDER_EXEC_MODE_ENABLE) == 0;
}

} // namespace v2