//===--------- queue_immediate_in_order.cpp - Level Zero Adapter ---------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../common.hpp"
#include "../queue.hpp"
#include "ur/ur.hpp"

namespace v2 {
struct ur_queue_immediate_in_order_t : _ur_object, public ur_queue_handle_t_ {
  ur_queue_immediate_in_order_t(ur_context_handle_t, ur_device_handle_t,
                                ur_queue_flags_t) {}

  ur_result_t getNativeHandle(ur_queue_native_desc_t *,
                              ur_native_handle_t *) override {
    return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
  }

  ur_result_t retain() override { return UR_RESULT_ERROR_UNSUPPORTED_FEATURE; }

  ur_result_t release() override { return UR_RESULT_ERROR_UNSUPPORTED_FEATURE; }
};

} // namespace v2
