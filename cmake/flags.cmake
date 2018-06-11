macro(configure_msvc_runtime)
	if(MSVC)
		# Default to statically-linked runtime.
		if("${MSVC_RUNTIME}" STREQUAL "")
			set(MSVC_RUNTIME "static")
		endif()
		# Set compiler options.
		set(variables
			CMAKE_C_FLAGS_DEBUG
			CMAKE_C_FLAGS_MINSIZEREL
			CMAKE_C_FLAGS_RELEASE
			CMAKE_C_FLAGS_RELWITHDEBINFO
			CMAKE_CXX_FLAGS_DEBUG
			CMAKE_CXX_FLAGS_MINSIZEREL
			CMAKE_CXX_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_RELWITHDEBINFO
		)
		if(${MSVC_RUNTIME} STREQUAL "static")
			message(STATUS "MSVC -> forcing use of statically-linked runtime.")
			foreach(variable ${variables})
				if(${variable} MATCHES "/MD")
					string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
				endif()
			endforeach()
		else()
		message(STATUS "MSVC -> forcing use of dynamically-linked runtime.")
		foreach(variable ${variables})
			if(${variable} MATCHES "/MT")
				string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
			endif()
		endforeach()
		endif()
	endif()
endmacro()

macro(print_compile_flags)
	set(variables
		CMAKE_C_FLAGS_DEBUG
		CMAKE_C_FLAGS_MINSIZEREL
		CMAKE_C_FLAGS_RELEASE
		CMAKE_C_FLAGS_RELWITHDEBINFO
		CMAKE_CXX_FLAGS_DEBUG
		CMAKE_CXX_FLAGS_MINSIZEREL
		CMAKE_CXX_FLAGS_RELEASE
		CMAKE_CXX_FLAGS_RELWITHDEBINFO
	)
	message(STATUS "Initial build flags:")
	foreach(variable ${variables})
		message(STATUS "  '${variable}': ${${variable}}")
	endforeach()
	message(STATUS "")
endmacro()

set(CMAKE_VERBOSE_MAKEFILE TRUE)

###############################################################################
# project directory
###############################################################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

###############################################################################
# output directories
###############################################################################
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/distro/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/distro/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/distro/bin)