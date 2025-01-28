#include "generator.hpp"
#include "callbacks.hpp"
int main() {
  auto static handle = enableTracing({}, {});
  std::cout << handle.get() << std::endl;
  initialize_cbs(handle.get());
}
