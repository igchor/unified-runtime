//===--------- image_ops.cpp - Level Zero Adapter ------------------------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "common.hpp"
#include "context.hpp"
#include "event.hpp"
#include "image.hpp"
#include "logger/ur_logger.hpp"
#include "sampler.hpp"
#include "ur_interface_loader.hpp"

static bool Is3ChannelOrder(ur_image_channel_order_t ChannelOrder) {
  switch (ChannelOrder) {
  case UR_IMAGE_CHANNEL_ORDER_RGB:
  case UR_IMAGE_CHANNEL_ORDER_RGX:
    return true;
  default:
    return false;
  }
}

namespace ur::level_zero {

ur_result_t urBindlessImagesImageCopyExp(
    ur_queue_handle_t hQueue, [[maybe_unused]] const void *pSrc,
    [[maybe_unused]] void *pDst,
    [[maybe_unused]] const ur_image_desc_t *pSrcImageDesc,
    [[maybe_unused]] const ur_image_desc_t *pDstImageDesc,
    [[maybe_unused]] const ur_image_format_t *pSrcImageFormat,
    [[maybe_unused]] const ur_image_format_t *pDstImageFormat,
    [[maybe_unused]] ur_exp_image_copy_region_t *pCopyRegion,
    [[maybe_unused]] ur_exp_image_copy_flags_t imageCopyFlags,
    [[maybe_unused]] uint32_t numEventsInWaitList,
    [[maybe_unused]] const ur_event_handle_t *phEventWaitList,
    [[maybe_unused]] ur_event_handle_t *phEvent) try {
  std::scoped_lock<ur_shared_mutex> Lock(hQueue->Mutex);

  UR_ASSERT(hQueue, UR_RESULT_ERROR_INVALID_NULL_HANDLE);
  UR_ASSERT(pDst && pSrc && pSrcImageFormat && pSrcImageDesc && pDstImageDesc &&
                pCopyRegion,
            UR_RESULT_ERROR_INVALID_NULL_POINTER);
  UR_ASSERT(pSrcImageDesc->type == pDstImageDesc->type,
            UR_RESULT_ERROR_INVALID_VALUE);
  UR_ASSERT(!(UR_EXP_IMAGE_COPY_FLAGS_MASK & imageCopyFlags),
            UR_RESULT_ERROR_INVALID_ENUMERATION);
  UR_ASSERT(!(pSrcImageDesc && UR_MEM_TYPE_IMAGE1D_ARRAY < pSrcImageDesc->type),
            UR_RESULT_ERROR_INVALID_IMAGE_FORMAT_DESCRIPTOR);

  bool UseCopyEngine = hQueue->useCopyEngine(/*PreferCopyEngine*/ true);
  // Due to the limitation of the copy engine, disable usage of Copy Engine
  // Given 3 channel image
  if (Is3ChannelOrder(
          ur_cast<ur_image_channel_order_t>(pSrcImageFormat->channelOrder)) ||
      Is3ChannelOrder(
          ur_cast<ur_image_channel_order_t>(pDstImageFormat->channelOrder))) {
    UseCopyEngine = false;
  }

  _ur_ze_event_list_t TmpWaitList;
  UR_CALL(TmpWaitList.createAndRetainUrZeEventList(
      numEventsInWaitList, phEventWaitList, hQueue, UseCopyEngine));

  bool Blocking = false;
  // We want to batch these commands to avoid extra submissions (costly)
  bool OkToBatch = true;

  // Get a new command list to be used on this call
  ur_command_list_ptr_t CommandList{};
  UR_CALL(hQueue->Context->getAvailableCommandList(
      hQueue, CommandList, UseCopyEngine, numEventsInWaitList, phEventWaitList,
      OkToBatch, nullptr /*ForcedCmdQueue*/));

  ze_event_handle_t ZeEvent = nullptr;
  ur_event_handle_t InternalEvent;
  bool IsInternal = phEvent == nullptr;
  ur_event_handle_t *Event = phEvent ? phEvent : &InternalEvent;
  UR_CALL(createEventAndAssociateQueue(hQueue, Event, UR_COMMAND_MEM_IMAGE_COPY,
                                       CommandList, IsInternal,
                                       /*IsMultiDevice*/ false));
  UR_CALL(setSignalEvent(hQueue, UseCopyEngine, &ZeEvent, Event,
                         numEventsInWaitList, phEventWaitList,
                         CommandList->second.ZeQueue));
  (*Event)->WaitList = TmpWaitList;

  const auto &ZeCommandList = CommandList->first;
  const auto &WaitList = (*Event)->WaitList;

  auto CopyDesc =
      getImageCopyDesc(imageCopyFlags, pSrcImageDesc, pSrcImageFormat,
                       pDstImageDesc, pDstImageFormat, pCopyRegion, pSrc, pDst);

  if (auto *Desc = std::get_if<h2d_non_usm>(&CopyDesc)) {
    auto *UrImage = reinterpret_cast<ur_exp_bindless_image_t *>(Desc->DstImage);
    ZE2UR_CALL(zeCommandListAppendImageCopyFromMemoryExt,
               (ZeCommandList, UrImage->getZeImage(), Desc->SrcPtr,
                &Desc->DstRegion, Desc->SrcRowPitch, Desc->SrcSlicePitch,
                ZeEvent, WaitList.Length, WaitList.ZeEventList));
  } else if (auto *Desc = std::get_if<h2d_pitched_usm>(&CopyDesc)) {
    ZE2UR_CALL(zeCommandListAppendMemoryCopyRegion,
               (ZeCommandList, Desc->DstPtr, &Desc->ZeDstRegion,
                Desc->DstRowPitch, Desc->DstSlicePitch, Desc->SrcPtr,
                &Desc->ZeSrcRegion, Desc->SrcRowPitch, Desc->SrcSlicePitch,
                ZeEvent, WaitList.Length, WaitList.ZeEventList));
  } else if (auto *Desc = std::get_if<d2h_non_usm>(&CopyDesc)) {
    auto *UrImage =
        reinterpret_cast<const ur_exp_bindless_image_t *>(Desc->SrcImage);
    ZE2UR_CALL(zeCommandListAppendImageCopyToMemoryExt,
               (ZeCommandList, Desc->DstPtr, UrImage->getZeImage(),
                &Desc->SrcRegion, Desc->DstRowPitch, Desc->DstSlicePitch,
                ZeEvent, WaitList.Length, WaitList.ZeEventList));
  } else if (auto *Desc = std::get_if<d2h_pitched_usm>(&CopyDesc)) {
    ZE2UR_CALL(zeCommandListAppendMemoryCopyRegion,
               (ZeCommandList, Desc->DstPtr, &Desc->ZeDstRegion,
                Desc->DstRowPitch, Desc->DstSlicePitch, Desc->SrcPtr,
                &Desc->ZeSrcRegion, Desc->SrcRowPitch, Desc->SrcSlicePitch,
                ZeEvent, WaitList.Length, WaitList.ZeEventList));
  } else if (auto *Desc = std::get_if<d2d>(&CopyDesc)) {
    auto SrcImage = reinterpret_cast<ur_exp_bindless_image_t *>(Desc->SrcImage);
    auto DstImage = reinterpret_cast<ur_exp_bindless_image_t *>(Desc->DstImage);
    ZE2UR_CALL(zeCommandListAppendImageCopyRegion,
               (ZeCommandList, DstImage->getZeImage(), SrcImage->getZeImage(),
                &Desc->SrcRegion, &Desc->DstRegion, ZeEvent, WaitList.Length,
                WaitList.ZeEventList));
  } else {
    logger::error("urBindlessImagesImageCopyExp: unexpected imageCopyFlags");
    throw UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
  }

  UR_CALL(hQueue->executeCommandList(CommandList, Blocking, OkToBatch));

  return UR_RESULT_SUCCESS;
} catch (...) {
  return exceptionToResult(std::current_exception());
}

ur_result_t urBindlessImagesWaitExternalSemaphoreExp(
    ur_queue_handle_t hQueue, ur_exp_external_semaphore_handle_t hSemaphore,
    bool hasValue, uint64_t waitValue, uint32_t numEventsInWaitList,
    const ur_event_handle_t *phEventWaitList, ur_event_handle_t *phEvent) {
  auto UrPlatform = hQueue->Context->getPlatform();
  if (UrPlatform->ZeExternalSemaphoreExt.Supported == false) {
    logger::error(logger::LegacyMessage("[UR][L0] "),
                  " {} function not supported!", __FUNCTION__);
    return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
  }

  bool UseCopyEngine = false;

  // We want to batch these commands to avoid extra submissions (costly)
  bool OkToBatch = true;

  _ur_ze_event_list_t TmpWaitList;
  UR_CALL(TmpWaitList.createAndRetainUrZeEventList(
      numEventsInWaitList, phEventWaitList, hQueue, UseCopyEngine));

  // Get a new command list to be used on this call
  ur_command_list_ptr_t CommandList{};
  UR_CALL(hQueue->Context->getAvailableCommandList(
      hQueue, CommandList, UseCopyEngine, numEventsInWaitList, phEventWaitList,
      OkToBatch, nullptr /*ForcedCmdQueue*/));

  ze_event_handle_t ZeEvent = nullptr;
  ur_event_handle_t InternalEvent;
  bool IsInternal = phEvent == nullptr;
  ur_event_handle_t *Event = phEvent ? phEvent : &InternalEvent;
  UR_CALL(createEventAndAssociateQueue(hQueue, Event,
                                       UR_COMMAND_EXTERNAL_SEMAPHORE_WAIT_EXP,
                                       CommandList, IsInternal,
                                       /*IsMultiDevice*/ false));
  UR_CALL(setSignalEvent(hQueue, UseCopyEngine, &ZeEvent, Event,
                         numEventsInWaitList, phEventWaitList,
                         CommandList->second.ZeQueue));
  (*Event)->WaitList = TmpWaitList;

  const auto &ZeCommandList = CommandList->first;
  const auto &WaitList = (*Event)->WaitList;

  ze_intel_external_semaphore_wait_exp_params_t WaitParams = {
      ZE_INTEL_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_WAIT_PARAMS_EXP, nullptr, 0};
  WaitParams.value = hasValue ? waitValue : 0;
  const ze_intel_external_semaphore_exp_handle_t hExtSemaphore =
      reinterpret_cast<ze_intel_external_semaphore_exp_handle_t>(hSemaphore);
  ZE2UR_CALL(UrPlatform->ZeExternalSemaphoreExt
                 .zexCommandListAppendWaitExternalSemaphoresExp,
             (ZeCommandList, 1, &hExtSemaphore, &WaitParams, ZeEvent,
              WaitList.Length, WaitList.ZeEventList));

  return UR_RESULT_SUCCESS;
}

ur_result_t urBindlessImagesSignalExternalSemaphoreExp(
    ur_queue_handle_t hQueue, ur_exp_external_semaphore_handle_t hSemaphore,
    bool hasValue, uint64_t signalValue, uint32_t numEventsInWaitList,
    const ur_event_handle_t *phEventWaitList, ur_event_handle_t *phEvent) {
  std::ignore = hQueue;
  std::ignore = hSemaphore;
  std::ignore = hasValue;
  std::ignore = signalValue;
  std::ignore = numEventsInWaitList;
  std::ignore = phEventWaitList;
  std::ignore = phEvent;
  auto UrPlatform = hQueue->Context->getPlatform();
  if (UrPlatform->ZeExternalSemaphoreExt.Supported == false) {
    logger::error(logger::LegacyMessage("[UR][L0] "),
                  " {} function not supported!", __FUNCTION__);
    return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
  }

  bool UseCopyEngine = false;

  // We want to batch these commands to avoid extra submissions (costly)
  bool OkToBatch = true;

  _ur_ze_event_list_t TmpWaitList;
  UR_CALL(TmpWaitList.createAndRetainUrZeEventList(
      numEventsInWaitList, phEventWaitList, hQueue, UseCopyEngine));

  // Get a new command list to be used on this call
  ur_command_list_ptr_t CommandList{};
  UR_CALL(hQueue->Context->getAvailableCommandList(
      hQueue, CommandList, UseCopyEngine, numEventsInWaitList, phEventWaitList,
      OkToBatch, nullptr /*ForcedCmdQueue*/));

  ze_event_handle_t ZeEvent = nullptr;
  ur_event_handle_t InternalEvent;
  bool IsInternal = phEvent == nullptr;
  ur_event_handle_t *Event = phEvent ? phEvent : &InternalEvent;
  UR_CALL(createEventAndAssociateQueue(hQueue, Event,
                                       UR_COMMAND_EXTERNAL_SEMAPHORE_SIGNAL_EXP,
                                       CommandList, IsInternal,
                                       /*IsMultiDevice*/ false));
  UR_CALL(setSignalEvent(hQueue, UseCopyEngine, &ZeEvent, Event,
                         numEventsInWaitList, phEventWaitList,
                         CommandList->second.ZeQueue));
  (*Event)->WaitList = TmpWaitList;

  const auto &ZeCommandList = CommandList->first;
  const auto &WaitList = (*Event)->WaitList;

  ze_intel_external_semaphore_signal_exp_params_t SignalParams = {
      ZE_INTEL_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_SIGNAL_PARAMS_EXP, nullptr, 0};
  SignalParams.value = hasValue ? signalValue : 0;
  const ze_intel_external_semaphore_exp_handle_t hExtSemaphore =
      reinterpret_cast<ze_intel_external_semaphore_exp_handle_t>(hSemaphore);

  ZE2UR_CALL(UrPlatform->ZeExternalSemaphoreExt
                 .zexCommandListAppendSignalExternalSemaphoresExp,
             (ZeCommandList, 1, &hExtSemaphore, &SignalParams, ZeEvent,
              WaitList.Length, WaitList.ZeEventList));

  return UR_RESULT_SUCCESS;
}

} // namespace ur::level_zero
