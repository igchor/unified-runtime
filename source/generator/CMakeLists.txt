project(generator)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

add_library(generator SHARED generator.cpp)
target_link_libraries(generator PRIVATE ze_loader)
include_directories(/usr/include/level_zero)
