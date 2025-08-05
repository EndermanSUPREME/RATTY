file(GLOB RATTY_SRC cpp/*.cpp)
file(GLOB RATTY_HEADER hpp/*.hpp)

file(GLOB UTILITIES_SRC ../utilities/cpp/*.cpp)
file(GLOB UTILITIES_HEADER ../utilities/hpp/*.hpp)

add_executable(ratty ${UTILITIES_SRC} ${UTILITIES_HEADER} ${RATTY_SRC} ${RATTY_HEADER})
# allow WINAPI usage
target_link_libraries(ratty PRIVATE gdi32 ws2_32)
# ensure the exe doesnt need to find dlls at runtime
target_compile_options(ratty PRIVATE
    -static-libgcc
    -static-libstdc++
    -mthreads
)
# mwindows ensures if the exe is ran via explorer
# we do not make a terminal window
target_link_options(ratty PRIVATE
    -static-libgcc
    -static-libstdc++
    #-mwindows
)