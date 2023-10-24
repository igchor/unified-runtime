/*
 *
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
 * See LICENSE.TXT
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* needed for dladdr */
#endif              /* _GNU_SOURCE */

#include <dlfcn.h>

#include "ur_lib.hpp"

namespace ur_lib {

void __attribute__((constructor)) createLibContext() {
        context = new context_t;
    Dl_info dlinfo;
    int res = dladdr(
        (void *)&urInit,
        &dlinfo);
    if (!res) {
        char const *err = dlerror();
        printf("%s\n", err);
        exit(1);
    }

    std::cout << "CONTEXT CTOR" << dlinfo.dli_fname << " " << context << " " << &context << std::endl;
}

void __attribute__((destructor)) deleteLibContext() { delete context; }

} // namespace ur_lib
