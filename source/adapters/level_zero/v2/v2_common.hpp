//===--------- common.cpp - Level Zero Adapter ---------------------------===//
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

#include <ze_api.h>

#include "logger/ur_logger.hpp"

namespace v2 {
namespace raii {

using ze_mem_handle_t = std::unique_ptr<void, std::function<void(void *)>>;

inline ze_mem_handle_t makeZeMemHandle(ze_context_handle_t hContext,
                                       void *ptr) {
  auto dtor = [hContext](void *ptr) {
    auto ret = zeMemFree(hContext, ptr);
    if (ret != ZE_RESULT_SUCCESS) {
      logger::error("zeMemFree failed: ", ret);
    }
  };
  return ze_mem_handle_t(ptr, std::move(dtor));
}

using ze_event_t = std::unique_ptr<ze_event_handle_t, decltype(&zeEventDestroy)>;

} // namespace raii
} // namespace v2
