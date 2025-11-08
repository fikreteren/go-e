cmake_minimum_required(VERSION 3.26)

project(ipc_task LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE IPC_TASK_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(ipc_task ${IPC_TASK_SOURCES})

target_include_directories(ipc_task PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/output")
set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/output")

set_target_properties(ipc_task PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

find_package(Threads REQUIRED)
target_link_libraries(ipc_task PRIVATE Threads::Threads)

find_package(fmt REQUIRED)
target_link_libraries(ipc_task PRIVATE fmt::fmt)

find_library(RT_LIB rt)
if (RT_LIB)
    target_link_libraries(ipc_task PRIVATE ${RT_LIB})
endif()

target_compile_options(ipc_task PRIVATE -Wall -Wextra -Wpedantic)

