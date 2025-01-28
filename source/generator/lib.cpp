#include "generator.hpp"
#include "callbacks.hpp"

void __attribute__((constructor)) Load() {
  auto static handle = enableTracing({}, {});
  std::cout << handle.get() << std::endl;
  initialize_cbs(handle.get());
  ZE_CALL(zelTracerSetEnabled(handle.get(), true));
}

void __attribute__((destructor)) Unload() {
}
