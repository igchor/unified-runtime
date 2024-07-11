//===--------- kernel.cpp - Level Zero Adapter ---------------------------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <ur_api.h>

#include "kernel.hpp"

namespace v2 {

ur_single_device_kernel_t::ur_single_device_kernel_t(ze_kernel_handle_t hKernel,
                                                     bool ownZeHandle)
    : hKernel(hKernel, ownZeHandle) {
  zeKernelProperties.Compute =
      [this, hKernel = hKernel](ze_kernel_properties_t &properties) {
        ZE_CALL_NOCHECK(zeKernelGetProperties, (hKernel, &properties));
      };
}

void ur_single_device_kernel_t::release() { hKernel.reset(); }

ur_kernel_handle_t_::ur_kernel_handle_t_(ur_program_handle_t hProgram,
                                         const char *kernelName)
    : hProgram(hProgram) {
  for (auto [zeDevice, zeModule] : hProgram->ZeModuleMap) {
    ZeStruct<ze_kernel_desc_t> zeKernelDesc;
    zeKernelDesc.pKernelName = kernelName;

    ze_kernel_handle_t zeKernel;
    ZE2UR_CALL_THROWS(zeKernelCreate, (zeModule, &zeKernelDesc, &zeKernel));

    deviceKernels.emplace_back(zeDevice,
                               ur_single_device_kernel_t(zeKernel, false));
  }
}

ur_kernel_handle_t_::ur_kernel_handle_t_(
    ur_native_handle_t hNativeKernel, ur_program_handle_t hProgram,
    const ur_kernel_native_properties_t *pProperties)
    : hProgram(hProgram) {
  ze_kernel_handle_t zeKernel = ur_cast<ze_kernel_handle_t>(hNativeKernel);
  deviceKernels.emplace_back(
      0, ur_single_device_kernel_t(zeKernel, pProperties->isNativeHandleOwned));
}

void ur_kernel_handle_t_::release() {
  for (auto &[_, singleDeviceKernel] : deviceKernels) {
    singleDeviceKernel.hKernel.reset();
  }

  // do a release on the program this kernel was part of without delete of the
  // program handle.
  hProgram->ur_release_program_resources(false);
}

void ur_kernel_handle_t_::completeInitialization() {
  // Retain the program to ensure it is not deleted before the kernel.
  UR_CALL_THROWS(urProgramRetain(hProgram));

  // Cache kernel name. Should be the same for all devices
  assert(deviceKernels.size() > 0);
  zeKernelName.Compute = [this](std::string &name) {
    size_t size = 0;
    ZE_CALL_NOCHECK(zeKernelGetName,
                    (deviceKernels[0].second.hKernel.get(), &size, nullptr));
    name.resize(size);
    ZE_CALL_NOCHECK(zeKernelGetName, (deviceKernels[0].second.hKernel.get(),
                                      &size, name.data()));
  };
}

ze_kernel_handle_t
ur_kernel_handle_t_::getZeHandle(ur_device_handle_t hDevice) {
  // root-device's kernel can be submitted to a sub-device's queue
  if (hDevice->isSubDevice()) {
    hDevice = hDevice->RootDevice;
  }

  if (deviceKernels.size() == 1) {
    assert(deviceKernels[0].first == hDevice->Id);
    return deviceKernels[0].second.hKernel.get();
  }

  auto it = std::find_if(
      deviceKernels.begin(), deviceKernels.end(),
      [hDevice](const auto &pair) { return pair.first == hDevice->Id; });

  if (it != deviceKernels.end()) {
    return it->second.hKernel.get();
  }

  return nullptr;
}

void ur_kernel_handle_t_::setArgValue(
    uint32_t argIndex, ///< [in] argument index in range [0, num args - 1]
    size_t argSize,    ///< [in] size of argument type
    const ur_kernel_arg_value_properties_t
        *pProperties, ///< [in][optional] argument properties
    const void
        *pArgValue ///< [in] argument value represented as matching arg type.
) {
  std::ignore = pProperties;

  // OpenCL: "the arg_value pointer can be NULL or point to a NULL value
  // in which case a NULL value will be used as the value for the argument
  // declared as a pointer to global or constant memory in the kernel"
  //
  // We don't know the type of the argument but it seems that the only time
  // SYCL RT would send a pointer to NULL in 'arg_value' is when the argument
  // is a NULL pointer. Treat a pointer to NULL in 'arg_value' as a NULL.
  if (argSize == sizeof(void *) && pArgValue &&
      *(void **)(const_cast<void *>(pArgValue)) == nullptr) {
    pArgValue = nullptr;
  }

  std::scoped_lock<ur_shared_mutex> guard(Mutex);

  for (auto &[_, singleDeviceKernel] : deviceKernels) {
    ZE2UR_CALL_THROWS(
        zeKernelSetArgumentValue,
        (singleDeviceKernel.hKernel.get(), argIndex, argSize, pArgValue));
  }
}

const std::string &ur_kernel_handle_t_::getName() const {
  return *zeKernelName.operator->();
}

const ze_kernel_properties_t &
ur_kernel_handle_t_::getProperties(ur_device_handle_t hDevice) const {
  auto it = std::find_if(
      deviceKernels.begin(), deviceKernels.end(),
      [hDevice](const auto &pair) { return pair.first == hDevice->Id; });

  if (it != deviceKernels.end()) {
    return *it->second.zeKernelProperties.operator->();
  }

  throw UR_RESULT_ERROR_INVALID_DEVICE;
}
} // namespace v2

UR_APIEXPORT ur_result_t UR_APICALL urKernelRetain(
    ur_kernel_handle_t Kernel ///< [in] handle for the Kernel to retain
) {
  Kernel->RefCount.increment();
  return UR_RESULT_SUCCESS;
}

UR_APIEXPORT ur_result_t UR_APICALL urKernelRelease(
    ur_kernel_handle_t hkernel ///< [in] handle for the Kernel to release
) {
  if (!hkernel->RefCount.decrementAndTest())
    return UR_RESULT_SUCCESS;

  hKernel->release();
  delete hkernel;

  return UR_RESULT_SUCCESS;
}