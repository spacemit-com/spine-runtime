cmake_minimum_required(VERSION 3.16)
project(spine_aidma_overlap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(spine-runtime CONFIG REQUIRED)

add_executable(spine_aidma_overlap overlap.cpp)
target_link_libraries(spine_aidma_overlap PRIVATE spine-runtime::spert)
