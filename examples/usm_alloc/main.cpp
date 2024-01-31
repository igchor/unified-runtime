#include <sycl/sycl.hpp>
#include <iostream>
#include <numeric>

int main() {
    sycl::queue q;
    auto *ptr = sycl::malloc_device<int>(1024, q);

    q.submit([&](sycl::handler &cgh) {
        cgh.parallel_for(1024, [=](sycl::id<1> WIid) {
          ptr[WIid.get(0)] = WIid.get(0);
        });
    });

    auto *out = sycl::malloc_shared<int>(1024, q);
    q.submit([&](sycl::handler &cgh) {
        cgh.parallel_for(1024, [=](sycl::id<1> WIid) {
          out[WIid.get(0)] = ptr[WIid.get(0)];
        });
    });

    std::cout << std::accumulate(out, out + 1024, 0) << std::endl;
    return 0;
}