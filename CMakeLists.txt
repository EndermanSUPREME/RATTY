cmake_minimum_required(VERSION 3.28)
project(ratty)

# enable ctest
enable_testing()

# target Windows 64-Bit when compiling
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# set std enforcement and compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wno-reorder -Wunused-variable -Wunused-parameter -Wnarrowing -Werror=narrowing -Wno-error=all")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(src/hpp)

# Add the source directory with its own CMakeLists.txt
add_subdirectory(src)