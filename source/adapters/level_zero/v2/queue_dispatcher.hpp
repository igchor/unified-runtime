//===--------- queue_dispatcher.hpp - Level Zero Adapter -----------------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include <variant>

#include "queue_immediate_in_order.hpp"

#include "../common.hpp"
#include "../device.hpp"
#include "ur/ur.hpp"

namespace v2 {
struct ur_queue_dispatcher_t : _ur_object {
  ur_queue_dispatcher_t(ur_context_handle_t Context, ur_device_handle_t Device,
                        ur_queue_flags_t Flags);

  ur_queue_dispatcher_t(const ur_queue_dispatcher_t &) = delete;
  ur_queue_dispatcher_t &operator=(const ur_queue_dispatcher_t &) = delete;

  void release();
  void retain();
private:
  std::variant<ur_queue_immediate_in_order_t> Queue;

  std::variant<ur_queue_immediate_in_order_t>
  initialize(ur_context_handle_t Context, ur_device_handle_t Device,
             ur_queue_flags_t Flags);
};

bool shouldUseDispatcher(ur_queue_flags_t Flags, ur_device_handle_t Device);

} // namespace v2

#define V2TryDispatch(UnifiedQueueHandle, Function)                            \
  {                                                                            \
    if (auto Dispatcher = std::get_if<v2::ur_queue_dispatcher_t>(              \
            &UnifiedQueueHandle->Queue)) {                                     \
      static_assert(std::is_same_v<void, decltype(Dispatcher->Function)>,       \
                    "Function must return void");                                \
      Dispatcher->Function;                                                    \
      return UR_RESULT_SUCCESS;                                                \
    }                                                                          \
  }
