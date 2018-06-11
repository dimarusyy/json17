project(unit-tests)
add_executable(unit-tests object-tests.cpp reader-tests.cpp)

target_link_libraries(boost)
if (clang_on_linux)
    target_link_libraries(unit_tests c++)
endif ()
