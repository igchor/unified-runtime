//===--------- common.hpp - Level Zero Adapter ---------------------------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <exception>
#include <ze_api.h>

#include "../common.hpp"

namespace v2 {

namespace raii {

template <typename ZeHandleT, ze_result_t (*destroy)(ZeHandleT)>
struct ze_handle_wrapper {
  ze_handle_wrapper(bool ownZeHandle = true)
      : handle(nullptr), ownZeHandle(ownZeHandle) {}

  ze_handle_wrapper(ZeHandleT handle, bool ownZeHandle = true)
      : handle(handle), ownZeHandle(ownZeHandle) {}

  ~ze_handle_wrapper() {
    try {
      reset();
    } catch (...) {
      logger::error("~ze_handle_wrapper: failed to release handle");
    }
  }

  void reset() {
    if (!handle) {
      return;
    }

    auto zeResult = ZE_CALL_NOCHECK(destroy, (handle));
    // Gracefully handle the case that L0 was already unloaded.
    if (zeResult && zeResult != ZE_RESULT_ERROR_UNINITIALIZED)
      throw ze2urResult(zeResult);

    handle = nullptr;
  }

  ZeHandle get() const { return handle; }

  ZeHandle *ptr() { return &handle; }

private:
  bool ownZeHandle;
  ZeHandle handle;
};

using ze_kernel_handle_t =
    ze_handle_wrapper<::ze_kernel_handle_t, zeKernelDestroy>;

} // namespace raii
} // namespace v2
