// Copyright (C) 2024 Intel Corporation
// Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
// See LICENSE.TXT
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <CL/sycl.hpp>

SYCL_EXTERNAL void this_function_does_not_exist();

int main() {
    cl::sycl::queue deviceQueue;
    cl::sycl::range<1> numOfItems{1};

    try {
        deviceQueue.submit([&](cl::sycl::handler &cgh) {
            auto kern = [=](cl::sycl::id<1>) {
#ifdef __SYCL_DEVICE_ONLY__
                this_function_does_not_exist();
#endif
            };
            cgh.parallel_for<class Foo>(numOfItems, kern);
        });
        std::cout << "Expected an error compiling the program." << std::endl;
        abort();
    } catch (sycl::exception &e) {
        // OK
    }

    return 0;
}
