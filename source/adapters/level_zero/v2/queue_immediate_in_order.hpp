//===--------- queue_immediate_in_order_t.cpp - Level Zero Adapter -------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../common.hpp"
#include "ur/ur.hpp"

namespace v2 {
struct ur_queue_immediate_in_order_t {
  ur_queue_immediate_in_order_t(ur_context_handle_t, ur_device_handle_t,
                                ur_queue_flags_t) {}
};
} // namespace v2