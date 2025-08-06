file(GLOB RAT_SIGNAL_SRC
    ../src/utilities/hpp/ratsignal.hpp
    ../src/utilities/cpp/ratsignal.cpp
    )

# Simple macro to reduce repetitive code
macro(add_unit_test TEST_FILE)
    get_filename_component(TEST_NAME_WITH_EXTENSION ${TEST_FILE} NAME)
    string(FIND ${TEST_NAME_WITH_EXTENSION} "." EXTENSION_BEGIN)
    string(SUBSTRING ${TEST_NAME_WITH_EXTENSION} 0 ${EXTENSION_BEGIN} TEST_NAME)

    add_executable(${TEST_NAME} ${RAT_SIGNAL_SRC} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} ws2_32)

    add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}>)
    set_target_properties(${TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    # ensure the exe doesnt need to find dlls at runtime
    target_compile_options(${TEST_NAME} PRIVATE
    -static-libgcc
    -static-libstdc++
    -mthreads
    )
    target_link_options(${TEST_NAME} PRIVATE
    -static-libgcc
    -static-libstdc++
    )
endmacro()

# ctest test items
add_unit_test(TestRatSignal.cpp)