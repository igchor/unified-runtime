//===--------- api.cpp - Level Zero Adapter ------------------------------===//
//
// Copyright (C) 2024 Intel Corporation
//
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM
// Exceptions. See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <mutex>
#include <ur_api.h>
#include <ze_api.h>

#include "../common.hpp"
#include "logger/ur_logger.hpp"

std::mutex ZeCall::GlobalLock;

namespace ur::level_zero {

ur_result_t
urContextSetExtendedDeleter(ur_context_handle_t hContext,
                            ur_context_extended_deleter_t pfnDeleter,
                            void *pUserData) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urKernelSetSpecializationConstants(
    ur_kernel_handle_t hKernel, uint32_t count,
    const ur_specialization_constant_info_t *pSpecConstants) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urEventSetCallback(ur_event_handle_t hEvent,
                               ur_execution_info_t execStatus,
                               ur_event_callback_t pfnNotify, void *pUserData) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t
urCommandBufferCreateExp(ur_context_handle_t hContext,
                         ur_device_handle_t hDevice,
                         const ur_exp_command_buffer_desc_t *pCommandBufferDesc,
                         ur_exp_command_buffer_handle_t *phCommandBuffer) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t
urCommandBufferRetainExp(ur_exp_command_buffer_handle_t hCommandBuffer) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t
urCommandBufferReleaseExp(ur_exp_command_buffer_handle_t hCommandBuffer) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t
urCommandBufferFinalizeExp(ur_exp_command_buffer_handle_t hCommandBuffer) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferAppendKernelLaunchExp(
    ur_exp_command_buffer_handle_t hCommandBuffer, ur_kernel_handle_t hKernel,
    uint32_t workDim, const size_t *pGlobalWorkOffset,
    const size_t *pGlobalWorkSize, const size_t *pLocalWorkSize,
    uint32_t numKernelAlternatives, ur_kernel_handle_t *phKernelAlternatives,
    uint32_t numSyncPointsInWaitList,
    const ur_exp_command_buffer_sync_point_t *pSyncPointWaitList,
    uint32_t NumEventsInWaitList, const ur_event_handle_t *phEventWaitList,
    ur_exp_command_buffer_sync_point_t *pSyncPoint, ur_event_handle_t *phEvent,
    ur_exp_command_buffer_command_handle_t *phCommand) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferAppendUSMMemcpyExp(
    ur_exp_command_buffer_handle_t hCommandBuffer, void *pDst, const void *pSrc,
    size_t size, uint32_t numSyncPointsInWaitList,
    const ur_exp_command_buffer_sync_point_t *pSyncPointWaitList,
    uint32_t NumEventsInWaitList, const ur_event_handle_t *phEventWaitList,
    ur_exp_command_buffer_sync_point_t *pSyncPoint, ur_event_handle_t *phEvent,
    ur_exp_command_buffer_command_handle_t *phCommand) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferAppendUSMFillExp(
    ur_exp_command_buffer_handle_t hCommandBuffer, void *pMemory,
    const void *pPattern, size_t patternSize, size_t size,
    uint32_t numSyncPointsInWaitList,
    const ur_exp_command_buffer_sync_point_t *pSyncPointWaitList,
    uint32_t NumEventsInWaitList, const ur_event_handle_t *phEventWaitList,
    ur_exp_command_buffer_sync_point_t *pSyncPoint, ur_event_handle_t *phEvent,
    ur_exp_command_buffer_command_handle_t *phCommand) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferAppendMemBufferCopyExp(
    ur_exp_command_buffer_handle_t hCommandBuffer, ur_mem_handle_t hSrcMem,
    ur_mem_handle_t hDstMem, size_t srcOffset, size_t dstOffset, size_t size,
    uint32_t numSyncPointsInWaitList,
    const ur_exp_command_buffer_sync_point_t *pSyncPointWaitList,
    uint32_t NumEventsInWaitList, const ur_event_handle_t *phEventWaitList,
    ur_exp_command_buffer_sync_point_t *pSyncPoint, ur_event_handle_t *phEvent,
    ur_exp_command_buffer_command_handle_t *phCommand) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferAppendMemBufferWriteExp(
    ur_exp_command_buffer_handle_t hCommandBuffer, ur_mem_handle_t hBuffer,
    size_t offset, size_t size, const void *pSrc,
    uint32_t numSyncPointsInWaitList,
    const ur_exp_command_buffer_sync_point_t *pSyncPointWaitList,
    uint32_t NumEventsInWaitList, const ur_event_handle_t *phEventWaitList,
    ur_exp_command_buffer_sync_point_t *pSyncPoint, ur_event_handle_t *phEvent,
    ur_exp_command_buffer_command_handle_t *phCommand) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferAppendMemBufferReadExp(
    ur_exp_command_buffer_handle_t hCommandBuffer, ur_mem_handle_t hBuffer,
    size_t offset, size_t size, void *pDst, uint32_t numSyncPointsInWaitList,
    const ur_exp_command_buffer_sync_point_t *pSyncPointWaitList,
    uint32_t NumEventsInWaitList, const ur_event_handle_t *phEventWaitList,
    ur_exp_command_buffer_sync_point_t *pSyncPoint, ur_event_handle_t *phEvent,
    ur_exp_command_buffer_command_handle_t *phCommand) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferAppendMemBufferCopyRectExp(
    ur_exp_command_buffer_handle_t hCommandBuffer, ur_mem_handle_t hSrcMem,
    ur_mem_handle_t hDstMem, ur_rect_offset_t srcOrigin,
    ur_rect_offset_t dstOrigin, ur_rect_region_t region, size_t srcRowPitch,
    size_t srcSlicePitch, size_t dstRowPitch, size_t dstSlicePitch,
    uint32_t numSyncPointsInWaitList,
    const ur_exp_command_buffer_sync_point_t *pSyncPointWaitList,
    uint32_t NumEventsInWaitList, const ur_event_handle_t *phEventWaitList,
    ur_exp_command_buffer_sync_point_t *pSyncPoint, ur_event_handle_t *phEvent,
    ur_exp_command_buffer_command_handle_t *phCommand) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferAppendMemBufferWriteRectExp(
    ur_exp_command_buffer_handle_t hCommandBuffer, ur_mem_handle_t hBuffer,
    ur_rect_offset_t bufferOffset, ur_rect_offset_t hostOffset,
    ur_rect_region_t region, size_t bufferRowPitch, size_t bufferSlicePitch,
    size_t hostRowPitch, size_t hostSlicePitch, void *pSrc,
    uint32_t numSyncPointsInWaitList,
    const ur_exp_command_buffer_sync_point_t *pSyncPointWaitList,
    uint32_t NumEventsInWaitList, const ur_event_handle_t *phEventWaitList,
    ur_exp_command_buffer_sync_point_t *pSyncPoint, ur_event_handle_t *phEvent,
    ur_exp_command_buffer_command_handle_t *phCommand) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferAppendMemBufferReadRectExp(
    ur_exp_command_buffer_handle_t hCommandBuffer, ur_mem_handle_t hBuffer,
    ur_rect_offset_t bufferOffset, ur_rect_offset_t hostOffset,
    ur_rect_region_t region, size_t bufferRowPitch, size_t bufferSlicePitch,
    size_t hostRowPitch, size_t hostSlicePitch, void *pDst,
    uint32_t numSyncPointsInWaitList,
    const ur_exp_command_buffer_sync_point_t *pSyncPointWaitList,
    uint32_t NumEventsInWaitList, const ur_event_handle_t *phEventWaitList,
    ur_exp_command_buffer_sync_point_t *pSyncPoint, ur_event_handle_t *phEvent,
    ur_exp_command_buffer_command_handle_t *phCommand) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferAppendMemBufferFillExp(
    ur_exp_command_buffer_handle_t hCommandBuffer, ur_mem_handle_t hBuffer,
    const void *pPattern, size_t patternSize, size_t offset, size_t size,
    uint32_t numSyncPointsInWaitList,
    const ur_exp_command_buffer_sync_point_t *pSyncPointWaitList,
    uint32_t NumEventsInWaitList, const ur_event_handle_t *phEventWaitList,
    ur_exp_command_buffer_sync_point_t *pSyncPoint, ur_event_handle_t *phEvent,
    ur_exp_command_buffer_command_handle_t *phCommand) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferAppendUSMPrefetchExp(
    ur_exp_command_buffer_handle_t hCommandBuffer, const void *pMemory,
    size_t size, ur_usm_migration_flags_t flags,
    uint32_t numSyncPointsInWaitList,
    const ur_exp_command_buffer_sync_point_t *pSyncPointWaitList,
    uint32_t NumEventsInWaitList, const ur_event_handle_t *phEventWaitList,
    ur_exp_command_buffer_sync_point_t *pSyncPoint, ur_event_handle_t *phEvent,
    ur_exp_command_buffer_command_handle_t *phCommand) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferAppendUSMAdviseExp(
    ur_exp_command_buffer_handle_t hCommandBuffer, const void *pMemory,
    size_t size, ur_usm_advice_flags_t advice, uint32_t numSyncPointsInWaitList,
    const ur_exp_command_buffer_sync_point_t *pSyncPointWaitList,
    uint32_t NumEventsInWaitList, const ur_event_handle_t *phEventWaitList,
    ur_exp_command_buffer_sync_point_t *pSyncPoint, ur_event_handle_t *phEvent,
    ur_exp_command_buffer_command_handle_t *phCommand) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferEnqueueExp(
    ur_exp_command_buffer_handle_t hCommandBuffer, ur_queue_handle_t hQueue,
    uint32_t numEventsInWaitList, const ur_event_handle_t *phEventWaitList,
    ur_event_handle_t *phEvent) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferRetainCommandExp(
    ur_exp_command_buffer_command_handle_t hCommand) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferReleaseCommandExp(
    ur_exp_command_buffer_command_handle_t hCommand) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferUpdateKernelLaunchExp(
    ur_exp_command_buffer_command_handle_t hCommand,
    const ur_exp_command_buffer_update_kernel_launch_desc_t
        *pUpdateKernelLaunch) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t
urCommandBufferGetInfoExp(ur_exp_command_buffer_handle_t hCommandBuffer,
                          ur_exp_command_buffer_info_t propName,
                          size_t propSize, void *pPropValue,
                          size_t *pPropSizeRet) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferUpdateSignalEventExp(
    ur_exp_command_buffer_command_handle_t hCommand,
    ur_event_handle_t *phEvent) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferUpdateWaitEventsExp(
    ur_exp_command_buffer_command_handle_t hCommand,
    uint32_t NumEventsInWaitList, const ur_event_handle_t *phEventWaitList) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ur_result_t urCommandBufferCommandGetInfoExp(
    ur_exp_command_buffer_command_handle_t hCommand,
    ur_exp_command_buffer_command_info_t propName, size_t propSize,
    void *pPropValue, size_t *pPropSizeRet) {
  logger::error("{} function not implemented!", __FUNCTION__);
  return UR_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

} // namespace ur::level_zero
