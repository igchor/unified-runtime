//===--------- memory_helpers.hpp - Level Zero Adapter -------------------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#pragma once

#include <ur_api.h>
#include <ze_api.h>

// If USM Import feature is enabled and hostptr is supplied,
// import the hostptr if not already imported into USM.
// Data transfer rate is maximized when both source and destination
// are USM pointers. Promotion of the host pointer to USM thus
// optimizes data transfer performance.
bool maybeImportUSM(ze_driver_handle_t hTranslatedDriver,
                    ze_context_handle_t hContext, void *ptr, size_t size);

ze_memory_type_t getMemoryType(ze_context_handle_t hContext, void *ptr);

struct ze_rect_params_t {
  uint32_t dstPitch;
  size_t dstSlicePitch;
  uint32_t srcPitch;
  size_t srcSlicePitch;
  ze_copy_region_t srcRegion;
  ze_copy_region_t dstRegion;
};

ze_rect_params_t ur2zeRectParams(ur_rect_offset_t SrcOrigin, ur_rect_offset_t DstOrigin,
    ur_rect_region_t Region, size_t SrcRowPitch, size_t DstRowPitch,
    size_t SrcSlicePitch, size_t DstSlicePitch);
