/*
 *
 * Copyright (C) 2021-2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 * @file trace_gen.h
 *
 */
#ifndef trace_gen_H
#define trace_gen_H

#include <layers/zel_tracing_register_cb.h>
#include "generator.hpp"

std::vector<std::string> get_assignments(ze_init_driver_type_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    return assignments;
}
std::vector<std::string> get_assignments(ze_context_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    return assignments;
}
std::vector<std::string> get_assignments(ze_command_queue_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("ordinal=" + desc_member_to_string(params.ordinal));
    assignments.push_back("index=" + desc_member_to_string(params.index));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    assignments.push_back("mode=" + desc_member_to_string(params.mode));
    assignments.push_back("priority=" + desc_member_to_string(params.priority));
    return assignments;
}
std::vector<std::string> get_assignments(ze_command_list_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("commandQueueGroupOrdinal=" + desc_member_to_string(params.commandQueueGroupOrdinal));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    return assignments;
}
std::vector<std::string> get_assignments(ze_event_pool_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    assignments.push_back("count=" + desc_member_to_string(params.count));
    return assignments;
}
std::vector<std::string> get_assignments(ze_event_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("index=" + desc_member_to_string(params.index));
    assignments.push_back("signal=" + desc_member_to_string(params.signal));
    assignments.push_back("wait=" + desc_member_to_string(params.wait));
    return assignments;
}
std::vector<std::string> get_assignments(ze_fence_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    return assignments;
}
std::vector<std::string> get_assignments(ze_image_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    assignments.push_back("type=" + desc_member_to_string(params.type));
    assignments.push_back("format=" + desc_member_to_string(params.format));
    assignments.push_back("width=" + desc_member_to_string(params.width));
    assignments.push_back("height=" + desc_member_to_string(params.height));
    assignments.push_back("depth=" + desc_member_to_string(params.depth));
    assignments.push_back("arraylevels=" + desc_member_to_string(params.arraylevels));
    assignments.push_back("miplevels=" + desc_member_to_string(params.miplevels));
    return assignments;
}
std::vector<std::string> get_assignments(ze_device_mem_alloc_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    assignments.push_back("ordinal=" + desc_member_to_string(params.ordinal));
    return assignments;
}
std::vector<std::string> get_assignments(ze_host_mem_alloc_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    return assignments;
}
std::vector<std::string> get_assignments(ze_module_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("format=" + desc_member_to_string(params.format));
    assignments.push_back("inputSize=" + desc_member_to_string(params.inputSize));
    assignments.push_back("pInputModule=" + desc_member_to_string(params.pInputModule));
    assignments.push_back("pBuildFlags=" + desc_member_to_string(params.pBuildFlags));
    assignments.push_back("pConstants=" + desc_member_to_string(params.pConstants));
    return assignments;
}
std::vector<std::string> get_assignments(ze_kernel_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    assignments.push_back("pKernelName=" + desc_member_to_string(params.pKernelName));
    return assignments;
}
std::vector<std::string> get_assignments(ze_sampler_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("addressMode=" + desc_member_to_string(params.addressMode));
    assignments.push_back("filterMode=" + desc_member_to_string(params.filterMode));
    assignments.push_back("isNormalized=" + desc_member_to_string(params.isNormalized));
    return assignments;
}
std::vector<std::string> get_assignments(ze_physical_mem_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    assignments.push_back("size=" + desc_member_to_string(params.size));
    return assignments;
}
std::vector<std::string> get_assignments(ze_scheduling_hint_exp_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    return assignments;
}
std::vector<std::string> get_assignments(ze_linkage_inspection_ext_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    return assignments;
}
std::vector<std::string> get_assignments(ze_memory_free_ext_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("freePolicy=" + desc_member_to_string(params.freePolicy));
    return assignments;
}
std::vector<std::string> get_assignments(ze_rtas_builder_build_op_exp_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("rtasFormat=" + desc_member_to_string(params.rtasFormat));
    assignments.push_back("buildQuality=" + desc_member_to_string(params.buildQuality));
    assignments.push_back("buildFlags=" + desc_member_to_string(params.buildFlags));
    assignments.push_back("ppGeometries=" + desc_member_to_string(params.ppGeometries));
    assignments.push_back("numGeometries=" + desc_member_to_string(params.numGeometries));
    return assignments;
}
std::vector<std::string> get_assignments(ze_mutable_command_id_exp_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    return assignments;
}
std::vector<std::string> get_assignments(ze_mutable_commands_exp_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    return assignments;
}
std::vector<std::string> get_assignments(ze_event_pool_counter_based_exp_desc_t params) {
    std::vector<std::string> assignments;
    assignments.push_back("stype=" + getZeStructureType(params.stype));
    assignments.push_back("flags=" + desc_member_to_string(params.flags));
    return assignments;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Container for all callbacks
void initialize_cbs(zel_tracer_handle_t handle)
{
    ZE_CALL(zelTracerInitRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeInit", make_args("zeInit",*params), result);
    }));
    ZE_CALL(zelTracerDriverGetRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDriverGet", make_args("zeDriverGet",*params), result);
    }));
    ZE_CALL(zelTracerInitDriversRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeInitDrivers", make_args("zeInitDrivers",*params), result);
    }));
    ZE_CALL(zelTracerDriverGetApiVersionRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDriverGetApiVersion", make_args("zeDriverGetApiVersion",*params), result);
    }));
    ZE_CALL(zelTracerDriverGetPropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDriverGetProperties", make_args("zeDriverGetProperties",*params), result);
    }));
    ZE_CALL(zelTracerDriverGetIpcPropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDriverGetIpcProperties", make_args("zeDriverGetIpcProperties",*params), result);
    }));
    ZE_CALL(zelTracerDriverGetExtensionPropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDriverGetExtensionProperties", make_args("zeDriverGetExtensionProperties",*params), result);
    }));
    ZE_CALL(zelTracerDriverGetExtensionFunctionAddressRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDriverGetExtensionFunctionAddress", make_args("zeDriverGetExtensionFunctionAddress",*params), result);
    }));
    ZE_CALL(zelTracerDriverGetLastErrorDescriptionRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDriverGetLastErrorDescription", make_args("zeDriverGetLastErrorDescription",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGet", make_args("zeDeviceGet",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetRootDeviceRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGetRootDevice", make_args("zeDeviceGetRootDevice",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetSubDevicesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGetSubDevices", make_args("zeDeviceGetSubDevices",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetPropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGetProperties", make_args("zeDeviceGetProperties",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetComputePropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGetComputeProperties", make_args("zeDeviceGetComputeProperties",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetModulePropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGetModuleProperties", make_args("zeDeviceGetModuleProperties",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetCommandQueueGroupPropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGetCommandQueueGroupProperties", make_args("zeDeviceGetCommandQueueGroupProperties",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetMemoryPropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGetMemoryProperties", make_args("zeDeviceGetMemoryProperties",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetMemoryAccessPropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGetMemoryAccessProperties", make_args("zeDeviceGetMemoryAccessProperties",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetCachePropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGetCacheProperties", make_args("zeDeviceGetCacheProperties",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetImagePropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGetImageProperties", make_args("zeDeviceGetImageProperties",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetExternalMemoryPropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGetExternalMemoryProperties", make_args("zeDeviceGetExternalMemoryProperties",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetP2PPropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGetP2PProperties", make_args("zeDeviceGetP2PProperties",*params), result);
    }));
    ZE_CALL(zelTracerDeviceCanAccessPeerRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceCanAccessPeer", make_args("zeDeviceCanAccessPeer",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetStatusRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGetStatus", make_args("zeDeviceGetStatus",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetGlobalTimestampsRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGetGlobalTimestamps", make_args("zeDeviceGetGlobalTimestamps",*params), result);
    }));
    ZE_CALL(zelTracerContextCreateRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeContextCreate", make_args("zeContextCreate",*params), result);
    }));
    ZE_CALL(zelTracerContextCreateExRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeContextCreateEx", make_args("zeContextCreateEx",*params), result);
    }));
    ZE_CALL(zelTracerContextDestroyRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeContextDestroy", make_args("zeContextDestroy",*params), result);
    }));
    ZE_CALL(zelTracerContextGetStatusRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeContextGetStatus", make_args("zeContextGetStatus",*params), result);
    }));
    ZE_CALL(zelTracerCommandQueueCreateRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandQueueCreate", make_args("zeCommandQueueCreate",*params), result);
    }));
    ZE_CALL(zelTracerCommandQueueDestroyRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandQueueDestroy", make_args("zeCommandQueueDestroy",*params), result);
    }));
    ZE_CALL(zelTracerCommandQueueExecuteCommandListsRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandQueueExecuteCommandLists", make_args("zeCommandQueueExecuteCommandLists",*params), result);
    }));
    ZE_CALL(zelTracerCommandQueueSynchronizeRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandQueueSynchronize", make_args("zeCommandQueueSynchronize",*params), result);
    }));
    ZE_CALL(zelTracerCommandQueueGetOrdinalRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandQueueGetOrdinal", make_args("zeCommandQueueGetOrdinal",*params), result);
    }));
    ZE_CALL(zelTracerCommandQueueGetIndexRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandQueueGetIndex", make_args("zeCommandQueueGetIndex",*params), result);
    }));
    ZE_CALL(zelTracerCommandListCreateRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListCreate", make_args("zeCommandListCreate",*params), result);
    }));
    ZE_CALL(zelTracerCommandListCreateImmediateRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListCreateImmediate", make_args("zeCommandListCreateImmediate",*params), result);
    }));
    ZE_CALL(zelTracerCommandListDestroyRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListDestroy", make_args("zeCommandListDestroy",*params), result);
    }));
    ZE_CALL(zelTracerCommandListCloseRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListClose", make_args("zeCommandListClose",*params), result);
    }));
    ZE_CALL(zelTracerCommandListResetRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListReset", make_args("zeCommandListReset",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendWriteGlobalTimestampRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendWriteGlobalTimestamp", make_args("zeCommandListAppendWriteGlobalTimestamp",*params), result);
    }));
    ZE_CALL(zelTracerCommandListHostSynchronizeRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListHostSynchronize", make_args("zeCommandListHostSynchronize",*params), result);
    }));
    ZE_CALL(zelTracerCommandListGetDeviceHandleRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListGetDeviceHandle", make_args("zeCommandListGetDeviceHandle",*params), result);
    }));
    ZE_CALL(zelTracerCommandListGetContextHandleRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListGetContextHandle", make_args("zeCommandListGetContextHandle",*params), result);
    }));
    ZE_CALL(zelTracerCommandListGetOrdinalRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListGetOrdinal", make_args("zeCommandListGetOrdinal",*params), result);
    }));
    ZE_CALL(zelTracerCommandListImmediateGetIndexRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListImmediateGetIndex", make_args("zeCommandListImmediateGetIndex",*params), result);
    }));
    ZE_CALL(zelTracerCommandListIsImmediateRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListIsImmediate", make_args("zeCommandListIsImmediate",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendBarrierRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendBarrier", make_args("zeCommandListAppendBarrier",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendMemoryRangesBarrierRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendMemoryRangesBarrier", make_args("zeCommandListAppendMemoryRangesBarrier",*params), result);
    }));
    ZE_CALL(zelTracerContextSystemBarrierRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeContextSystemBarrier", make_args("zeContextSystemBarrier",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendMemoryCopyRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendMemoryCopy", make_args("zeCommandListAppendMemoryCopy",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendMemoryFillRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendMemoryFill", make_args("zeCommandListAppendMemoryFill",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendMemoryCopyRegionRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendMemoryCopyRegion", make_args("zeCommandListAppendMemoryCopyRegion",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendMemoryCopyFromContextRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendMemoryCopyFromContext", make_args("zeCommandListAppendMemoryCopyFromContext",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendImageCopyRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendImageCopy", make_args("zeCommandListAppendImageCopy",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendImageCopyRegionRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendImageCopyRegion", make_args("zeCommandListAppendImageCopyRegion",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendImageCopyToMemoryRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendImageCopyToMemory", make_args("zeCommandListAppendImageCopyToMemory",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendImageCopyFromMemoryRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendImageCopyFromMemory", make_args("zeCommandListAppendImageCopyFromMemory",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendMemoryPrefetchRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendMemoryPrefetch", make_args("zeCommandListAppendMemoryPrefetch",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendMemAdviseRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendMemAdvise", make_args("zeCommandListAppendMemAdvise",*params), result);
    }));
    ZE_CALL(zelTracerEventPoolCreateRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventPoolCreate", make_args("zeEventPoolCreate",*params), result);
    }));
    ZE_CALL(zelTracerEventPoolDestroyRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventPoolDestroy", make_args("zeEventPoolDestroy",*params), result);
    }));
    ZE_CALL(zelTracerEventCreateRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventCreate", make_args("zeEventCreate",*params), result);
    }));
    ZE_CALL(zelTracerEventDestroyRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventDestroy", make_args("zeEventDestroy",*params), result);
    }));
    ZE_CALL(zelTracerEventPoolGetIpcHandleRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventPoolGetIpcHandle", make_args("zeEventPoolGetIpcHandle",*params), result);
    }));
    ZE_CALL(zelTracerEventPoolPutIpcHandleRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventPoolPutIpcHandle", make_args("zeEventPoolPutIpcHandle",*params), result);
    }));
    ZE_CALL(zelTracerEventPoolOpenIpcHandleRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventPoolOpenIpcHandle", make_args("zeEventPoolOpenIpcHandle",*params), result);
    }));
    ZE_CALL(zelTracerEventPoolCloseIpcHandleRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventPoolCloseIpcHandle", make_args("zeEventPoolCloseIpcHandle",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendSignalEventRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendSignalEvent", make_args("zeCommandListAppendSignalEvent",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendWaitOnEventsRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendWaitOnEvents", make_args("zeCommandListAppendWaitOnEvents",*params), result);
    }));
    ZE_CALL(zelTracerEventHostSignalRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventHostSignal", make_args("zeEventHostSignal",*params), result);
    }));
    ZE_CALL(zelTracerEventHostSynchronizeRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventHostSynchronize", make_args("zeEventHostSynchronize",*params), result);
    }));
    ZE_CALL(zelTracerEventQueryStatusRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventQueryStatus", make_args("zeEventQueryStatus",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendEventResetRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendEventReset", make_args("zeCommandListAppendEventReset",*params), result);
    }));
    ZE_CALL(zelTracerEventHostResetRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventHostReset", make_args("zeEventHostReset",*params), result);
    }));
    ZE_CALL(zelTracerEventQueryKernelTimestampRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventQueryKernelTimestamp", make_args("zeEventQueryKernelTimestamp",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendQueryKernelTimestampsRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendQueryKernelTimestamps", make_args("zeCommandListAppendQueryKernelTimestamps",*params), result);
    }));
    ZE_CALL(zelTracerEventGetEventPoolRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventGetEventPool", make_args("zeEventGetEventPool",*params), result);
    }));
    ZE_CALL(zelTracerEventGetSignalScopeRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventGetSignalScope", make_args("zeEventGetSignalScope",*params), result);
    }));
    ZE_CALL(zelTracerEventGetWaitScopeRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventGetWaitScope", make_args("zeEventGetWaitScope",*params), result);
    }));
    ZE_CALL(zelTracerEventPoolGetContextHandleRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventPoolGetContextHandle", make_args("zeEventPoolGetContextHandle",*params), result);
    }));
    ZE_CALL(zelTracerEventPoolGetFlagsRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventPoolGetFlags", make_args("zeEventPoolGetFlags",*params), result);
    }));
    ZE_CALL(zelTracerFenceCreateRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeFenceCreate", make_args("zeFenceCreate",*params), result);
    }));
    ZE_CALL(zelTracerFenceDestroyRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeFenceDestroy", make_args("zeFenceDestroy",*params), result);
    }));
    ZE_CALL(zelTracerFenceHostSynchronizeRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeFenceHostSynchronize", make_args("zeFenceHostSynchronize",*params), result);
    }));
    ZE_CALL(zelTracerFenceQueryStatusRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeFenceQueryStatus", make_args("zeFenceQueryStatus",*params), result);
    }));
    ZE_CALL(zelTracerFenceResetRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeFenceReset", make_args("zeFenceReset",*params), result);
    }));
    ZE_CALL(zelTracerImageGetPropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeImageGetProperties", make_args("zeImageGetProperties",*params), result);
    }));
    ZE_CALL(zelTracerImageCreateRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeImageCreate", make_args("zeImageCreate",*params), result);
    }));
    ZE_CALL(zelTracerImageDestroyRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeImageDestroy", make_args("zeImageDestroy",*params), result);
    }));
    ZE_CALL(zelTracerMemAllocSharedRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemAllocShared", make_args("zeMemAllocShared",*params), result);
    }));
    ZE_CALL(zelTracerMemAllocDeviceRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemAllocDevice", make_args("zeMemAllocDevice",*params), result);
    }));
    ZE_CALL(zelTracerMemAllocHostRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemAllocHost", make_args("zeMemAllocHost",*params), result);
    }));
    ZE_CALL(zelTracerMemFreeRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemFree", make_args("zeMemFree",*params), result);
    }));
    ZE_CALL(zelTracerMemGetAllocPropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemGetAllocProperties", make_args("zeMemGetAllocProperties",*params), result);
    }));
    ZE_CALL(zelTracerMemGetAddressRangeRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemGetAddressRange", make_args("zeMemGetAddressRange",*params), result);
    }));
    ZE_CALL(zelTracerMemGetIpcHandleRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemGetIpcHandle", make_args("zeMemGetIpcHandle",*params), result);
    }));
    ZE_CALL(zelTracerMemGetIpcHandleFromFileDescriptorExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemGetIpcHandleFromFileDescriptorExp", make_args("zeMemGetIpcHandleFromFileDescriptorExp",*params), result);
    }));
    ZE_CALL(zelTracerMemGetFileDescriptorFromIpcHandleExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemGetFileDescriptorFromIpcHandleExp", make_args("zeMemGetFileDescriptorFromIpcHandleExp",*params), result);
    }));
    ZE_CALL(zelTracerMemPutIpcHandleRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemPutIpcHandle", make_args("zeMemPutIpcHandle",*params), result);
    }));
    ZE_CALL(zelTracerMemOpenIpcHandleRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemOpenIpcHandle", make_args("zeMemOpenIpcHandle",*params), result);
    }));
    ZE_CALL(zelTracerMemCloseIpcHandleRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemCloseIpcHandle", make_args("zeMemCloseIpcHandle",*params), result);
    }));
    ZE_CALL(zelTracerMemSetAtomicAccessAttributeExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemSetAtomicAccessAttributeExp", make_args("zeMemSetAtomicAccessAttributeExp",*params), result);
    }));
    ZE_CALL(zelTracerMemGetAtomicAccessAttributeExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemGetAtomicAccessAttributeExp", make_args("zeMemGetAtomicAccessAttributeExp",*params), result);
    }));
    ZE_CALL(zelTracerModuleCreateRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeModuleCreate", make_args("zeModuleCreate",*params), result);
    }));
    ZE_CALL(zelTracerModuleDestroyRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeModuleDestroy", make_args("zeModuleDestroy",*params), result);
    }));
    ZE_CALL(zelTracerModuleDynamicLinkRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeModuleDynamicLink", make_args("zeModuleDynamicLink",*params), result);
    }));
    ZE_CALL(zelTracerModuleBuildLogDestroyRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeModuleBuildLogDestroy", make_args("zeModuleBuildLogDestroy",*params), result);
    }));
    ZE_CALL(zelTracerModuleBuildLogGetStringRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeModuleBuildLogGetString", make_args("zeModuleBuildLogGetString",*params), result);
    }));
    ZE_CALL(zelTracerModuleGetNativeBinaryRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeModuleGetNativeBinary", make_args("zeModuleGetNativeBinary",*params), result);
    }));
    ZE_CALL(zelTracerModuleGetGlobalPointerRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeModuleGetGlobalPointer", make_args("zeModuleGetGlobalPointer",*params), result);
    }));
    ZE_CALL(zelTracerModuleGetKernelNamesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeModuleGetKernelNames", make_args("zeModuleGetKernelNames",*params), result);
    }));
    ZE_CALL(zelTracerModuleGetPropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeModuleGetProperties", make_args("zeModuleGetProperties",*params), result);
    }));
    ZE_CALL(zelTracerKernelCreateRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeKernelCreate", make_args("zeKernelCreate",*params), result);
    }));
    ZE_CALL(zelTracerKernelDestroyRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeKernelDestroy", make_args("zeKernelDestroy",*params), result);
    }));
    ZE_CALL(zelTracerModuleGetFunctionPointerRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeModuleGetFunctionPointer", make_args("zeModuleGetFunctionPointer",*params), result);
    }));
    ZE_CALL(zelTracerKernelSetGroupSizeRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeKernelSetGroupSize", make_args("zeKernelSetGroupSize",*params), result);
    }));
    ZE_CALL(zelTracerKernelSuggestGroupSizeRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeKernelSuggestGroupSize", make_args("zeKernelSuggestGroupSize",*params), result);
    }));
    ZE_CALL(zelTracerKernelSuggestMaxCooperativeGroupCountRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeKernelSuggestMaxCooperativeGroupCount", make_args("zeKernelSuggestMaxCooperativeGroupCount",*params), result);
    }));
    ZE_CALL(zelTracerKernelSetArgumentValueRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeKernelSetArgumentValue", make_args("zeKernelSetArgumentValue",*params), result);
    }));
    ZE_CALL(zelTracerKernelSetIndirectAccessRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeKernelSetIndirectAccess", make_args("zeKernelSetIndirectAccess",*params), result);
    }));
    ZE_CALL(zelTracerKernelGetIndirectAccessRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeKernelGetIndirectAccess", make_args("zeKernelGetIndirectAccess",*params), result);
    }));
    ZE_CALL(zelTracerKernelGetSourceAttributesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeKernelGetSourceAttributes", make_args("zeKernelGetSourceAttributes",*params), result);
    }));
    ZE_CALL(zelTracerKernelSetCacheConfigRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeKernelSetCacheConfig", make_args("zeKernelSetCacheConfig",*params), result);
    }));
    ZE_CALL(zelTracerKernelGetPropertiesRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeKernelGetProperties", make_args("zeKernelGetProperties",*params), result);
    }));
    ZE_CALL(zelTracerKernelGetNameRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeKernelGetName", make_args("zeKernelGetName",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendLaunchKernelRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendLaunchKernel", make_args("zeCommandListAppendLaunchKernel",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendLaunchCooperativeKernelRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendLaunchCooperativeKernel", make_args("zeCommandListAppendLaunchCooperativeKernel",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendLaunchKernelIndirectRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendLaunchKernelIndirect", make_args("zeCommandListAppendLaunchKernelIndirect",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendLaunchMultipleKernelsIndirectRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendLaunchMultipleKernelsIndirect", make_args("zeCommandListAppendLaunchMultipleKernelsIndirect",*params), result);
    }));
    ZE_CALL(zelTracerContextMakeMemoryResidentRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeContextMakeMemoryResident", make_args("zeContextMakeMemoryResident",*params), result);
    }));
    ZE_CALL(zelTracerContextEvictMemoryRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeContextEvictMemory", make_args("zeContextEvictMemory",*params), result);
    }));
    ZE_CALL(zelTracerContextMakeImageResidentRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeContextMakeImageResident", make_args("zeContextMakeImageResident",*params), result);
    }));
    ZE_CALL(zelTracerContextEvictImageRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeContextEvictImage", make_args("zeContextEvictImage",*params), result);
    }));
    ZE_CALL(zelTracerSamplerCreateRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeSamplerCreate", make_args("zeSamplerCreate",*params), result);
    }));
    ZE_CALL(zelTracerSamplerDestroyRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeSamplerDestroy", make_args("zeSamplerDestroy",*params), result);
    }));
    ZE_CALL(zelTracerVirtualMemReserveRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeVirtualMemReserve", make_args("zeVirtualMemReserve",*params), result);
    }));
    ZE_CALL(zelTracerVirtualMemFreeRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeVirtualMemFree", make_args("zeVirtualMemFree",*params), result);
    }));
    ZE_CALL(zelTracerVirtualMemQueryPageSizeRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeVirtualMemQueryPageSize", make_args("zeVirtualMemQueryPageSize",*params), result);
    }));
    ZE_CALL(zelTracerPhysicalMemCreateRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zePhysicalMemCreate", make_args("zePhysicalMemCreate",*params), result);
    }));
    ZE_CALL(zelTracerPhysicalMemDestroyRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zePhysicalMemDestroy", make_args("zePhysicalMemDestroy",*params), result);
    }));
    ZE_CALL(zelTracerVirtualMemMapRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeVirtualMemMap", make_args("zeVirtualMemMap",*params), result);
    }));
    ZE_CALL(zelTracerVirtualMemUnmapRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeVirtualMemUnmap", make_args("zeVirtualMemUnmap",*params), result);
    }));
    ZE_CALL(zelTracerVirtualMemSetAccessAttributeRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeVirtualMemSetAccessAttribute", make_args("zeVirtualMemSetAccessAttribute",*params), result);
    }));
    ZE_CALL(zelTracerVirtualMemGetAccessAttributeRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeVirtualMemGetAccessAttribute", make_args("zeVirtualMemGetAccessAttribute",*params), result);
    }));
    ZE_CALL(zelTracerKernelSetGlobalOffsetExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeKernelSetGlobalOffsetExp", make_args("zeKernelSetGlobalOffsetExp",*params), result);
    }));
    ZE_CALL(zelTracerKernelGetBinaryExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeKernelGetBinaryExp", make_args("zeKernelGetBinaryExp",*params), result);
    }));
    ZE_CALL(zelTracerDeviceReserveCacheExtRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceReserveCacheExt", make_args("zeDeviceReserveCacheExt",*params), result);
    }));
    ZE_CALL(zelTracerDeviceSetCacheAdviceExtRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceSetCacheAdviceExt", make_args("zeDeviceSetCacheAdviceExt",*params), result);
    }));
    ZE_CALL(zelTracerEventQueryTimestampsExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventQueryTimestampsExp", make_args("zeEventQueryTimestampsExp",*params), result);
    }));
    ZE_CALL(zelTracerImageGetMemoryPropertiesExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeImageGetMemoryPropertiesExp", make_args("zeImageGetMemoryPropertiesExp",*params), result);
    }));
    ZE_CALL(zelTracerImageViewCreateExtRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeImageViewCreateExt", make_args("zeImageViewCreateExt",*params), result);
    }));
    ZE_CALL(zelTracerImageViewCreateExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeImageViewCreateExp", make_args("zeImageViewCreateExp",*params), result);
    }));
    ZE_CALL(zelTracerKernelSchedulingHintExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeKernelSchedulingHintExp", make_args("zeKernelSchedulingHintExp",*params), result);
    }));
    ZE_CALL(zelTracerDevicePciGetPropertiesExtRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDevicePciGetPropertiesExt", make_args("zeDevicePciGetPropertiesExt",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendImageCopyToMemoryExtRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendImageCopyToMemoryExt", make_args("zeCommandListAppendImageCopyToMemoryExt",*params), result);
    }));
    ZE_CALL(zelTracerCommandListAppendImageCopyFromMemoryExtRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListAppendImageCopyFromMemoryExt", make_args("zeCommandListAppendImageCopyFromMemoryExt",*params), result);
    }));
    ZE_CALL(zelTracerImageGetAllocPropertiesExtRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeImageGetAllocPropertiesExt", make_args("zeImageGetAllocPropertiesExt",*params), result);
    }));
    ZE_CALL(zelTracerModuleInspectLinkageExtRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeModuleInspectLinkageExt", make_args("zeModuleInspectLinkageExt",*params), result);
    }));
    ZE_CALL(zelTracerMemFreeExtRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemFreeExt", make_args("zeMemFreeExt",*params), result);
    }));
    ZE_CALL(zelTracerFabricVertexGetExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeFabricVertexGetExp", make_args("zeFabricVertexGetExp",*params), result);
    }));
    ZE_CALL(zelTracerFabricVertexGetSubVerticesExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeFabricVertexGetSubVerticesExp", make_args("zeFabricVertexGetSubVerticesExp",*params), result);
    }));
    ZE_CALL(zelTracerFabricVertexGetPropertiesExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeFabricVertexGetPropertiesExp", make_args("zeFabricVertexGetPropertiesExp",*params), result);
    }));
    ZE_CALL(zelTracerFabricVertexGetDeviceExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeFabricVertexGetDeviceExp", make_args("zeFabricVertexGetDeviceExp",*params), result);
    }));
    ZE_CALL(zelTracerDeviceGetFabricVertexExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDeviceGetFabricVertexExp", make_args("zeDeviceGetFabricVertexExp",*params), result);
    }));
    ZE_CALL(zelTracerFabricEdgeGetExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeFabricEdgeGetExp", make_args("zeFabricEdgeGetExp",*params), result);
    }));
    ZE_CALL(zelTracerFabricEdgeGetVerticesExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeFabricEdgeGetVerticesExp", make_args("zeFabricEdgeGetVerticesExp",*params), result);
    }));
    ZE_CALL(zelTracerFabricEdgeGetPropertiesExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeFabricEdgeGetPropertiesExp", make_args("zeFabricEdgeGetPropertiesExp",*params), result);
    }));
    ZE_CALL(zelTracerEventQueryKernelTimestampsExtRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeEventQueryKernelTimestampsExt", make_args("zeEventQueryKernelTimestampsExt",*params), result);
    }));
    ZE_CALL(zelTracerRTASBuilderCreateExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeRTASBuilderCreateExp", make_args("zeRTASBuilderCreateExp",*params), result);
    }));
    ZE_CALL(zelTracerRTASBuilderGetBuildPropertiesExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeRTASBuilderGetBuildPropertiesExp", make_args("zeRTASBuilderGetBuildPropertiesExp",*params), result);
    }));
    ZE_CALL(zelTracerDriverRTASFormatCompatibilityCheckExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeDriverRTASFormatCompatibilityCheckExp", make_args("zeDriverRTASFormatCompatibilityCheckExp",*params), result);
    }));
    ZE_CALL(zelTracerRTASBuilderBuildExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeRTASBuilderBuildExp", make_args("zeRTASBuilderBuildExp",*params), result);
    }));
    ZE_CALL(zelTracerRTASBuilderDestroyExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeRTASBuilderDestroyExp", make_args("zeRTASBuilderDestroyExp",*params), result);
    }));
    ZE_CALL(zelTracerRTASParallelOperationCreateExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeRTASParallelOperationCreateExp", make_args("zeRTASParallelOperationCreateExp",*params), result);
    }));
    ZE_CALL(zelTracerRTASParallelOperationGetPropertiesExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeRTASParallelOperationGetPropertiesExp", make_args("zeRTASParallelOperationGetPropertiesExp",*params), result);
    }));
    ZE_CALL(zelTracerRTASParallelOperationJoinExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeRTASParallelOperationJoinExp", make_args("zeRTASParallelOperationJoinExp",*params), result);
    }));
    ZE_CALL(zelTracerRTASParallelOperationDestroyExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeRTASParallelOperationDestroyExp", make_args("zeRTASParallelOperationDestroyExp",*params), result);
    }));
    ZE_CALL(zelTracerMemGetPitchFor2dImageRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeMemGetPitchFor2dImage", make_args("zeMemGetPitchFor2dImage",*params), result);
    }));
    ZE_CALL(zelTracerImageGetDeviceOffsetExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeImageGetDeviceOffsetExp", make_args("zeImageGetDeviceOffsetExp",*params), result);
    }));
    ZE_CALL(zelTracerCommandListCreateCloneExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListCreateCloneExp", make_args("zeCommandListCreateCloneExp",*params), result);
    }));
    ZE_CALL(zelTracerCommandListImmediateAppendCommandListsExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListImmediateAppendCommandListsExp", make_args("zeCommandListImmediateAppendCommandListsExp",*params), result);
    }));
    ZE_CALL(zelTracerCommandListGetNextCommandIdExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListGetNextCommandIdExp", make_args("zeCommandListGetNextCommandIdExp",*params), result);
    }));
    ZE_CALL(zelTracerCommandListGetNextCommandIdWithKernelsExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListGetNextCommandIdWithKernelsExp", make_args("zeCommandListGetNextCommandIdWithKernelsExp",*params), result);
    }));
    ZE_CALL(zelTracerCommandListUpdateMutableCommandsExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListUpdateMutableCommandsExp", make_args("zeCommandListUpdateMutableCommandsExp",*params), result);
    }));
    ZE_CALL(zelTracerCommandListUpdateMutableCommandSignalEventExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListUpdateMutableCommandSignalEventExp", make_args("zeCommandListUpdateMutableCommandSignalEventExp",*params), result);
    }));
    ZE_CALL(zelTracerCommandListUpdateMutableCommandWaitEventsExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListUpdateMutableCommandWaitEventsExp", make_args("zeCommandListUpdateMutableCommandWaitEventsExp",*params), result);
    }));
    ZE_CALL(zelTracerCommandListUpdateMutableCommandKernelsExpRegisterCallback(handle, ZEL_REGISTER_EPILOGUE, [](auto params, ze_result_t result, void*, void**) {
        return ze_cb_invoke("zeCommandListUpdateMutableCommandKernelsExp", make_args("zeCommandListUpdateMutableCommandKernelsExp",*params), result);
    }));
};

#endif // trace_gen_H
