project(unit-tests)
enable_testing()

# sources
set(SOURCES stdafx.h stdafx.cpp object-tests.cpp reader-tests.cpp)

add_executable(unit-tests ${SOURCES})
target_link_libraries(unit-tests ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
if (clang_on_linux)
    target_link_libraries(unit_tests c++)
endif ()
