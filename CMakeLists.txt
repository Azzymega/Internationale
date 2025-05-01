cmake_minimum_required(VERSION 3.28)

project(ltos C ASM)

set(CMAKE_C_STANDARD 99)

include_directories(include)

add_subdirectory(intermarx)
add_subdirectory(ltoskrnl)

target_link_libraries(ltoskrnl PUBLIC intermarx)