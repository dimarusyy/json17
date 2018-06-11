################################################## 
# using vcpkg ?
################################################## 
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	if (EXISTS "$ENV{CMAKE_TOOLCHAIN_FILE}")
		set(CMAKE_TOOLCHAIN_FILE "$ENV{CMAKE_TOOLCHAIN_FILE}")
		message("-- Using vcpkg cmake integration : CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
		include(${CMAKE_TOOLCHAIN_FILE})
	else()
		message("-- Skipping vcpkg integration : CMAKE_TOOLCHAIN_FILE=[$ENV{CMAKE_TOOLCHAIN_FILE}]")
	endif ()
endif ()

###############################################################################
# Boost
###############################################################################
find_package(Boost COMPONENTS)
if (Boost_INCLUDE_DIRS)
  add_library(boost INTERFACE)
  target_include_directories(boost INTERFACE ${Boost_INCLUDE_DIRS})
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
