# Copyright (C) 2022-2023 Intel Corporation
# SPDX-License-Identifier: MIT

add_library(test_c STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/test.c
)

add_executable(test_main test_main.cpp)
target_link_libraries(test_main
    PRIVATE
    test_c)