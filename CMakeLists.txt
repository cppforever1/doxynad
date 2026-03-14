cmake_minimum_required(VERSION 3.10.0)
project(doxynad VERSION 0.1.0 LANGUAGES C CXX)

# make it cpp20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add spdlog dependency from github
include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)

FetchContent_MakeAvailable(spdlog)
include_directories(${spdlog_INCLUDE_DIRS})

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

add_executable(doxynad
    main.cpp
)

target_link_libraries(doxynad
    PRIVATE
        spdlog::spdlog
        fmt::fmt
)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
