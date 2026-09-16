cmake_minimum_required(VERSION 3.16)
project(spine_quickstart LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(spine-runtime CONFIG REQUIRED)

add_executable(spine_quickstart demo.cpp)
target_link_libraries(spine_quickstart PRIVATE spine-runtime::spert)
