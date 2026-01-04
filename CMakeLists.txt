cmake_minimum_required(VERSION 3.16)
project(solstice)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

cmake_policy(SET CMP0167 OLD)

# Find Boost (required by Boost.Beast for WebSocket)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

include(FetchContent)

find_package(Python COMPONENTS Interpreter Development REQUIRED)

FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11
  GIT_TAG v2.13.6
)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

FetchContent_MakeAvailable(pybind11 googletest)

add_subdirectory(src)
add_subdirectory(tests)

enable_testing()
