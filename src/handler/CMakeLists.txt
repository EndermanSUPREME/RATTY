file(GLOB HANDLER_SRC cpp/*.cpp)
file(GLOB HANDLER_HEADER headers/*.hpp)

file(GLOB UTILITIES_SRC ../utilities/cpp/*.cpp)
file(GLOB UTILITIES_HEADER ../utilities/hpp/*.hpp)

add_executable(ratconsole ${UTILITIES_SRC} ${UTILITIES_HEADER} ${HANDLER_SRC} ${HANDLER_HEADER})
# allow WINAPI usage
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