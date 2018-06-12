################################################## 
# using vcpkg ?
################################################## 
if (EXISTS "$ENV{VCPKG_TARGET_TRIPLET}")
	set(VCPKG_TARGET_TRIPLET " $ENV{VCPKG_TARGET_TRIPLET}" CACHE STRING "")
	message("-- Using vcpkg triplet(env) : ${VCPKG_TARGET_TRIPLET}")
else()
	if (MSVC)
		if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
			set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
		else()
			set(VCPKG_TARGET_TRIPLET "x86-windows-static" CACHE STRING "")
		endif()
	endif()
	message("-- Using vcpkg triplet : VCPKG_TARGET_TRIPLET=[${VCPKG_TARGET_TRIPLET}]")
endif()

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	if (EXISTS "$ENV{CMAKE_TOOLCHAIN_FILE}")
		set(CMAKE_TOOLCHAIN_FILE "$ENV{CMAKE_TOOLCHAIN_FILE}" CACHE STRING "")
		message("-- Using vcpkg cmake integration(env) : CMAKE_TOOLCHAIN_FILE=[${CMAKE_TOOLCHAIN_FILE}]")
		include(${CMAKE_TOOLCHAIN_FILE})
	else()
		message("-- Skipping vcpkg integration : CMAKE_TOOLCHAIN_FILE=[${CMAKE_TOOLCHAIN_FILE}]")
	endif ()
else()
	message("-- Using vcpkg toolchain : CMAKE_TOOLCHAIN_FILE=[${CMAKE_TOOLCHAIN_FILE}]")
	include(${CMAKE_TOOLCHAIN_FILE})
endif ()

###############################################################################
# Boost
###############################################################################
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
if (Boost_FOUND)
	add_library(boost INTERFACE)
	include_directories(${Boost_INCLUDE_DIRS})
else ()
	message("-- Boost was not found; attempting to download it if we haven't already...")
	include(ExternalProject)
	ExternalProject_Add(install-Boost
	    PREFIX ${CMAKE_BINARY_DIR}/dependencies/boost_1_67_0
		URL https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.bz2
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
		LOG_DOWNLOAD ON
	)

	ExternalProject_Get_Property(install-Boost SOURCE_DIR)
	add_library(boost INTERFACE)
	target_include_directories(boost INTERFACE ${SOURCE_DIR})
	add_dependencies(boost install-Boost)
	unset(SOURCE_DIR)
endif ()
