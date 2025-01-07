//===--------- image.hpp - Level Zero Adapter ----------------------------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#pragma once

#include <ur/ur.hpp>
#include <ze_api.h>
#include <zes_api.h>

#include "common.hpp"
#include "v2/common.hpp"

/// Construct UR image format from ZE image desc.
ur_result_t ze2urImageFormat(const ze_image_desc_t *ZeImageDesc,
                             ur_image_format_t *UrImageFormat);

/// Construct ZE image desc from UR image format and desc.
ur_result_t ur2zeImageDesc(const ur_image_format_t *ImageFormat,
                           const ur_image_desc_t *ImageDesc,
                           ZeStruct<ze_image_desc_t> &ZeImageDesc);

ur_result_t getImageRegionHelper(ze_image_desc_t ZeImageDesc,
                                 ur_rect_offset_t *Origin,
                                 ur_rect_region_t *Region,
                                 ze_image_region_t &ZeRegion);

struct h2d_non_usm {
  const void *SrcPtr;
  ur_exp_image_native_handle_t DstImage;

  uint32_t SrcRowPitch;
  uint32_t SrcSlicePitch;
  ze_image_region_t DstRegion;
};

struct h2d_pitched_usm {
  const void *SrcPtr;
  void *DstPtr;

  uint32_t SrcRowPitch;
  uint32_t SrcSlicePitch;

  uint32_t DstRowPitch;
  ze_copy_region_t ZeDstRegion;
  uint32_t DstSlicePitch;
  ze_copy_region_t ZeSrcRegion;
};

struct d2h_non_usm {
  ur_exp_image_native_handle_t SrcImage;
  void *DstPtr;

  ze_image_region_t SrcRegion;
  uint32_t DstRowPitch;
  uint32_t DstSlicePitch;
};

struct d2h_pitched_usm {
  const void *SrcPtr;
  void *DstPtr;

  uint32_t SrcRowPitch;
  ze_copy_region_t ZeSrcRegion;
  uint32_t SrcSlicePitch;

  uint32_t DstRowPitch;
  ze_copy_region_t ZeDstRegion;
  uint32_t DstSlicePitch;
};

struct d2d {
  ur_exp_image_native_handle_t SrcImage;
  ur_exp_image_native_handle_t DstImage;

  ze_image_region_t SrcRegion;
  ze_image_region_t DstRegion;
};

using image_copy_desc_t = std::variant<h2d_non_usm, h2d_pitched_usm,
                                       d2h_non_usm, d2h_pitched_usm, d2d>;

image_copy_desc_t getImageCopyDesc(ur_exp_image_copy_flags_t imageCopyFlags,
                                   const ur_image_desc_t *pSrcImageDesc,
                                   const ur_image_format_t *pSrcImageFormat,
                                   const ur_image_desc_t *pDstImageDesc,
                                   const ur_image_format_t *pDstImageFormat,
                                   ur_exp_image_copy_region_t *pCopyRegion,
                                   const void *pSrc, void *pDst);

struct ur_exp_bindless_image_t : public _ur_object {
  struct info_t {
    uint64_t width;
    uint64_t height;
    uint32_t depth;
    ur_image_format_t format;
  };

  ur_exp_bindless_image_t(ze_image_handle_t hImage, info_t info)
      : zeImage(hImage), info(info) {}

  info_t getInfo() const { return info; }

  ze_image_handle_t getZeImage() const { return zeImage.get(); }

private:
  v2::raii::ze_image_handle_t zeImage;
  const info_t info;
};
