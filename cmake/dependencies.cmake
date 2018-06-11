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
    PREFIX ${CMAKE_BINARY_DIR}/dependencies/boost_1_64_0
    URL https://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.tar.bz2
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
