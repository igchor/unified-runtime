# Copyright (C) 2022-2023 Intel Corporation
# Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
# See LICENSE.TXT
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

add_ur_library(ur_common STATIC
    umf_helpers.hpp
    umf_pools/disjoint_pool_config_parser.cpp
    ur_pool_manager.hpp
    ur_util.cpp
    ur_util.hpp
    $<$<PLATFORM_ID:Windows>:windows/ur_lib_loader.cpp>
    $<$<PLATFORM_ID:Linux,Darwin>:linux/ur_lib_loader.cpp>
)
add_library(${PROJECT_NAME}::common ALIAS ur_common)

target_include_directories(ur_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

message(STATUS "Download Unified Memory Framework from github.com")
if (NOT DEFINED UMF_REPO)
    set(UMF_REPO "https://github.com/oneapi-src/unified-memory-framework.git")
endif()

if (NOT DEFINED UMF_TAG)
    # Merge pull request #320 from ldorau/Enable_the_proxy_library_on_Windows
    set(UMF_TAG fad47c36924bf676a30abaccc053a4da7efcf6e9)
endif()

message(STATUS "Will fetch Unified Memory Framework from ${UMF_REPO}")

include(FetchContent)
FetchContent_Declare(unified-memory-framework
    GIT_REPOSITORY    ${UMF_REPO}
    GIT_TAG           ${UMF_TAG}
)

if(UR_BUILD_TESTS)
    set(UMF_BUILD_TESTS ON CACHE INTERNAL "Build UMF tests")
else()
    set(UMF_BUILD_TESTS OFF CACHE INTERNAL "Build UMF tests")
endif()
# TODO: L0 provider not ready yet
set(UMF_BUILD_LEVEL_ZERO_PROVIDER OFF CACHE INTERNAL "Build Level Zero Provider")
set(UMF_BUILD_SHARED_LIBRARY ON CACHE INTERNAL "Build UMF shared library")
set(UMF_BUILD_LIBUMF_POOL_DISJOINT ON CACHE INTERNAL "Build Disjoint Pool")

FetchContent_MakeAvailable(unified-memory-framework)
FetchContent_GetProperties(unified-memory-framework)

target_link_libraries(ur_common PUBLIC
    unified-memory-framework::umf
    unified-memory-framework::headers
    unified-memory-framework::disjoint_pool
    ${CMAKE_DL_LIBS}
    ${PROJECT_NAME}::headers
)

if (UNIX)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)
    target_link_libraries(ur_common PUBLIC Threads::Threads)
endif()
