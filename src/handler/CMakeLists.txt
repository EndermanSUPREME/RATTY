cmake_minimum_required(VERSION 3.28)
project(ratconsole)

if (WIN32)
    # target Windows 64-Bit when compiling
    set(CMAKE_SYSTEM_NAME Windows)
    set(CMAKE_SYSTEM_PROCESSOR x86_64)
    set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
elseif(UNIX)
    # target Unix when compiling
    set(CMAKE_CXX_COMPILER g++)
endif()

# set std enforcement and compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wno-reorder -Wunused-variable -Wunused-parameter -Wnarrowing -Werror=narrowing -Wno-error=all")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# when looking for includes on compile we want
# to search folders we store hpp files
include_directories(
    hpp
    ../utilities/hpp
    ../server/hpp
    ../../external
    )

# Download the CLI11 parser into a global directory for other executables to use
# (no point to have cli11.hpp in multiple locations)
set(CMAKE_EXTERNAL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../external)
# Name of file is needed since content of file itself are being downloaded.
file(DOWNLOAD https://github.com/CLIUtils/CLI11/releases/download/v2.3.2/CLI11.hpp ${CMAKE_EXTERNAL_SOURCE_DIR}/CLI11.hpp)

# Need to include in the exe different files based on OS target
# Windows handles Sockets differently than Unix
# The compilers also use different options as well
file(GLOB HANDLER_SRC cpp/*.cpp)
file(GLOB HANDLER_HEADER hpp/*.hpp)

file(GLOB SERVER_SRC ../server/cpp/*.cpp)
file(GLOB SERVER_HEADER ../server/hpp/*.hpp)

file(GLOB UTILITIES_SRC ../utilities/cpp/*.cpp)
file(GLOB UTILITIES_HEADER ../utilities/hpp/*.hpp)

add_executable(ratconsole ${SERVER_SRC} ${SERVER_HEADER} ${UTILITIES_SRC} ${UTILITIES_HEADER} ${HANDLER_SRC} ${HANDLER_HEADER})

if (WIN32)
    target_link_libraries(ratconsole PRIVATE gdi32 ws2_32)
    # ensure the exe doesnt need to find dlls at runtime
    target_compile_options(ratconsole PRIVATE
        -static-libgcc
        -static-libstdc++
        -mthreads
    )
    target_link_options(ratconsole PRIVATE
        -static-libgcc
        -static-libstdc++
    )
elseif(UNIX)
    # target Unix when compiling
    target_link_libraries(ratconsole PRIVATE pthread)
endif()