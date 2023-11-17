# Copyright (C) 2022-2023 Intel Corporation
# Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
# See LICENSE.TXT
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

message(STATUS "Download Unified Memory Framework from github.com")

set(UMF_REPO "https://github.com/oneapi-src/unified-memory-framework.git")

# Merge pull request #30 from igchor/cpp_helpers
set(UMF_TAG 64d36ca6b28834f8d4a6e1d3c77e0e19767ba228)

message(STATUS "Will fetch Unified Memory Framework from ${UMF_REPO}")

include(FetchContent)
FetchContent_Declare(unified-memory-framework
    GIT_REPOSITORY    ${UMF_REPO}
    GIT_TAG           ${UMF_TAG}
)

FetchContent_MakeAvailable(unified-memory-framework)
FetchContent_GetProperties(unified-memory-framework)

add_library(ur_common INTERFACE)
add_library(${PROJECT_NAME}::common ALIAS ur_common)

target_include_directories(ur_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(ur_common INTERFACE unified_memory_framework ${CMAKE_DL_LIBS} ${PROJECT_NAME}::headers)

if(WIN32)
    target_sources(ur_common
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/windows/ur_lib_loader.cpp
            umf_helpers.hpp ur_pool_manager.hpp umf_pools/disjoint_pool_config_parser.cpp
    )
else()
    target_sources(ur_common
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/linux/ur_lib_loader.cpp
            umf_helpers.hpp ur_pool_manager.hpp umf_pools/disjoint_pool_config_parser.cpp
    )
endif()
