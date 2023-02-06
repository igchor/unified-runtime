# Copyright (C) 2022-2023 Intel Corporation
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)
project(unified-runtime VERSION 0.5.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CXX flags setup
if(NOT MSVC)
    add_compile_options(-fPIC -Wall)
    if(UR_DEVELOPER_MODE)
        add_compile_options(-fno-omit-frame-pointer)
        add_compile_options(-Werror)
    endif()
elseif(MSVC)
    string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    add_compile_options(/MP /W3)

    if(UR_DEVELOPER_MODE)
        add_compile_options(/WX)
    endif()
endif()

add_subdirectory(test/test_c_cpp)
