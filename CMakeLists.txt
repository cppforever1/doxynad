cmake_minimum_required(VERSION 3.10.0)
project(doxynad VERSION 0.1.0 LANGUAGES C CXX)

# make it cpp20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(doxynad main.cpp)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
