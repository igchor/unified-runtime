//===--------- kernel.hpp - Level Zero Adapter ---------------------------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include "../memory.hpp"
#include "common.hpp"

namespace v2 {

struct ur_single_device_kernel_t {
  ur_single_device_kernel_t(ze_kernel_handle_t hKernel, bool ownZeHandle);

  void release();

  raii::ze_kernel_handle_t hKernel;
  mutable ZeCache<ZeStruct<ze_kernel_properties_t>> zeKernelProperties;
};

struct ur_kernel_handle_t_ : _ur_object {
  ur_kernel_handle_t_(ur_program_handle_t hProgram, const char *kernelName);

  // From native handle
  ur_kernel_handle_t_(ur_native_handle_t hNativeKernel,
                      ur_program_handle_t hProgram,
                      const ur_kernel_native_properties_t *pProperties);

  // Keep the program of the kernel.
  const ur_program_handle_t hProgram;

  // Get L0 kernel handle for a given device
  ze_kernel_handle_t getZeHandle(ur_device_handle_t hDevice);

  // Get name of the kernel.
  const std::string &getName() const;

  // Get properties of the kernel.
  const ze_kernel_properties_t &getProperties(ur_device_handle_t hDevice) const;

  // Implementation of urKernelSetArgValue.
  void setArgValue(uint32_t ArgIndex, size_t ArgSize,
                   const ur_kernel_arg_value_properties_t *Properties,
                   const void *PArgValue);

  // Perform cleanup.
  void release();

private:
  // Vector of <device_id, ur_single_device_kernel_t> pairs.
  std::vector<std::pair<size_t, ur_single_device_kernel_t>> deviceKernels;

  // Cache of the kernel name.
  mutable ZeCache<std::string> zeKernelName;

  void completeInitialization();
};

} // namespace v2
