# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: MIT

add_library(unified_memory_allocation STATIC
    src/memory_pool.c
    src/memory_provider.c
)

target_include_directories(unified_memory_allocation PUBLIC include)
