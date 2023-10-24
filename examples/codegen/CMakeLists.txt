# Copyright (C) 2022 Intel Corporation
# Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
# See LICENSE.TXT
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(TARGET_NAME codegen)


    add_ur_executable(${TARGET_NAME}
        ${CMAKE_CURRENT_SOURCE_DIR}/codegen.cpp
    )

    target_link_libraries(${TARGET_NAME}
        PRIVATE
            ${CMAKE_DL_LIBS}
            ${PROJECT_NAME}::headers
            ${PROJECT_NAME}::loader
    )

