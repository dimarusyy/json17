cmake_minimum_required(VERSION 3.5)
project(json17)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

################################################## 
# C++ standard version selection
################################################## 
function(constexpr_if_std std_flag var)
    try_compile(
        worked
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/constexpr_if.cpp
        COMPILE_DEFINITIONS ${std_flag} -Wall -Werror -DCHECK_CONSTEXPR_IF=1
    )
    set(${var} ${worked} PARENT_SCOPE)
endfunction ()

function(try_std_flag std_flag)
    try_compile(
        std_supported
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/constexpr_if.cpp
        COMPILE_DEFINITIONS ${std_flag} -Wall -Werror -DCHECK_CONSTEXPR_IF=0
    )
    if (std_supported)
        message("-- Checking compiler flag ${std_flag} -- success")
        set(std_flag ${std_flag} PARENT_SCOPE)
        constexpr_if_std(${std_flag} have_constexpr_if)
        if (have_constexpr_if)
            set(constexpr_if_define -DBOOST_NO_CONSTEXPR_IF=0 PARENT_SCOPE)
            message("-- Checking constexpr if support -- success")
        else ()
            set(constexpr_if_define -DBOOST_NO_CONSTEXPR_IF=1 PARENT_SCOPE)
            message("-- Checking constexpr if support -- failed to compile")
        endif ()
    else ()
        message("-- Checking compiler flag ${std_flag} -- failed to compile")
    endif ()
endfunction ()

if (NOT MSVC)
	try_std_flag(-std=c++17)
	if (NOT std_flag)
		try_std_flag(-std=c++1z) 
	elseif (NOT std_flag)
		try_std_flag(-std=c++14)
	elseif (NOT std_flag)
		message(FATAL_ERROR "Only Clang or GCC with -std=c++14 or later will work")
	endif ()
endif ()

##################################################
# Clang+Linux support
##################################################
set(clang_on_linux false)
if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    add_definitions(${std_flag} -stdlib=libc++ -g -Wall)
    if (CMAKE_SYSTEM_NAME STREQUAL Linux)
        set(clang_on_linux true)
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    add_definitions(${std_flag} -g -Wall)
else ()
	if (NOT MSVC)
		message(FATAL_ERROR "Only Clang or GCC with -std=c++14 or later will work")
	endif()
endif ()

##################################################
# Dependencies
##################################################
include(dependencies)
include(flags)

##################################################
# Flags
##################################################
configure_msvc_runtime()
print_compile_flags()

##################################################
# Subdirectories
##################################################
add_subdirectory(json17)
add_subdirectory(unit-tests)
