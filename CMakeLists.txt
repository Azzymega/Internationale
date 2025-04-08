cmake_minimum_required(VERSION 3.28)

project(ltos C ASM)

include_directories(include)

set(CMAKE_C_STANDARD 99)

add_subdirectory(ltoskrnl)
