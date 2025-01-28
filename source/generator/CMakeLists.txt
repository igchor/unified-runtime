project(generator)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

add_library(generator SHARED lib.cpp)
target_link_libraries(generator PRIVATE ze_loader)
include_directories(/usr/include/level_zero)

#add_executable(test test.cpp)
#target_link_libraries(test PRIVATE ze_loader)
