# Copyright (C) 2022-2024 Intel Corporation
# Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
# See LICENSE.TXT
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

if(UR_BUILD_ADAPTER_L0)
    add_ur_adapter(ur_adapter_level_zero
        SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/ur_interface_loader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/command_buffer.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/command_buffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/context.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/device.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/event.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/usm.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/memory.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/physical_mem.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platform.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/program.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/queue_api.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/queue.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sampler.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/helpers/kernel_helpers.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ur_level_zero.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/device.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/enqueue_native.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/event.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/usm.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/usm_p2p.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/virtual_mem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/memory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/kernel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/physical_mem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platform.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/program.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/queue_api.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/queue.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sampler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/image.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/helpers/kernel_helpers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../ur/ur.cpp
    )

    if(NOT WIN32)
        target_sources(ur_adapter_level_zero
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/adapter_lib_init_linux.cpp
        )
    endif()

    # TODO: fix level_zero adapter conversion warnings
    target_compile_options(ur_adapter_level_zero PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/wd4805 /wd4244>
    )

    set_target_properties(ur_adapter_level_zero PROPERTIES
        VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
        SOVERSION "${PROJECT_VERSION_MAJOR}"
    )

    if (WIN32)
    # 0x800: Search for the DLL only in the System32 folder
    target_link_options(ur_adapter_level_zero PUBLIC /DEPENDENTLOADFLAG:0x800)
    endif()

    target_link_libraries(ur_adapter_level_zero PRIVATE
        ${PROJECT_NAME}::headers
        ${PROJECT_NAME}::common
        ${PROJECT_NAME}::umf
        LevelZeroLoader
        LevelZeroLoader-Headers
    )

    target_include_directories(ur_adapter_level_zero PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../../"
        LevelZeroLoader-Headers
    )
endif()

if(UR_BUILD_ADAPTER_L0_V2)
    add_ur_adapter(ur_adapter_level_zero_v2
        SHARED
        # sources shared with legacy adapter
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/device.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platform.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/program.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/helpers/kernel_helpers.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/device.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ur_interface_loader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platform.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/program.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/helpers/kernel_helpers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../ur/ur.cpp
        # v2-only sources
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/command_list_cache.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/context.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/event_pool_cache.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/event_pool.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/event_provider_counter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/event_provider_normal.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/event_provider.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/event.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/kernel.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/queue_api.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/queue_immediate_in_order.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/usm.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/api.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/command_list_cache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/event_pool_cache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/event_pool.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/event_provider_counter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/event_provider_normal.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/event.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/kernel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/queue_api.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/queue_create.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/queue_immediate_in_order.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/v2/usm.cpp
    )

    if(NOT WIN32)
        # api.cpp contains NOT_SUPPORTED functions-only
        set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/v2/api.cpp
            PROPERTIES APPEND_STRING PROPERTY COMPILE_FLAGS "-Wno-unused-parameter")

        target_sources(ur_adapter_level_zero_v2
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/adapter_lib_init_linux.cpp
        )
    endif()

    target_compile_definitions(ur_adapter_level_zero_v2 PUBLIC UR_ADAPTER_LEVEL_ZERO_V2)

    # TODO: fix level_zero adapter conversion warnings
    target_compile_options(ur_adapter_level_zero_v2 PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/wd4805 /wd4244>
    )

    set_target_properties(ur_adapter_level_zero_v2 PROPERTIES
        VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
        SOVERSION "${PROJECT_VERSION_MAJOR}"
    )

    if (WIN32)
    # 0x800: Search for the DLL only in the System32 folder
    target_link_options(ur_adapter_level_zero_v2 PUBLIC /DEPENDENTLOADFLAG:0x800)
    endif()

    target_link_libraries(ur_adapter_level_zero_v2 PRIVATE
        ${PROJECT_NAME}::headers
        ${PROJECT_NAME}::common
        ${PROJECT_NAME}::umf
        LevelZeroLoader
        LevelZeroLoader-Headers
    )

    target_include_directories(ur_adapter_level_zero_v2 PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../.."
        "${CMAKE_CURRENT_SOURCE_DIR}/../../ur"
        LevelZeroLoader-Headers
    )
endif()
