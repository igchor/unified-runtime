#include "cuda_fixtures.h"

using urCudaGetDeviceNativeHandle = uur::urDeviceTest;
UUR_INSTANTIATE_DEVICE_TEST_SUITE_P(urCudaGetDeviceNativeHandle);

TEST_P(urCudaGetDeviceNativeHandle, Success) {
    ur_native_handle_t native_handle;
    ASSERT_SUCCESS(urDeviceGetNativeHandle(device, &native_handle));

    CUdevice cuda_device = *reinterpret_cast<CUdevice *>(&native_handle);

    char cuda_device_name[256];
    ASSERT_SUCCESS_CUDA(cuDeviceGetName(cuda_device_name,
                                        sizeof(cuda_device_name), cuda_device));
}