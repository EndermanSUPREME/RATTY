file(GLOB RATTY_SRC cpp/*.cpp)
file(GLOB RATTY_HEADER headers/*.hpp)

add_executable(ratty ${RATTY_SRC} ${RATTY_HEADER})
# allow WINAPI usage
target_link_libraries(ratty PRIVATE gdi32)
# ensure the exe doesnt need to find dlls at runtime
target_compile_options(ratty PRIVATE -static-libgcc -static-libstdc++)
target_link_options(ratty PRIVATE -static-libgcc -static-libstdc++)