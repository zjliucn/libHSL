###############################################################################
# Main CMake configuration file for libHSL
#
# Author: Mateusz Loskot <mateusz@loskot.net>
# Author: Zhengjun Liu <zjliu@casm.ac.cn>
#
#
###############################################################################

# libHSL general settings
project(libHSL)


SET(LIBHSL_VERSION_MAJOR "0")
SET(LIBHSL_VERSION_MINOR "1")
SET(LIBHSL_VERSION_PATCH "0")
set(VERSION "${LIBHSL_VERSION_MAJOR}.${LIBHSL_VERSION_MINOR}.${LIBHSL_VERSION_PATCH}")
SET(LIBHSL_LIB_SOVERSION "0.1.0")
SET(LIBHSL_C_LIB_SOVERSION "0.1.0")
SET(OSGEO4W_UPSTREAM_RELEASE "1")

if(WIN32)
  # Name of C++ library
  set(LIBHSL_LIB_NAME libhsl)

  # Name of C library
  set(LIBHSL_C_LIB_NAME libhsl_c)
  
  # Name of Chipper library
  set(HSLCHIP_LIB_NAME libhsl_chipper )  
  
  # Name of Kernel libray
  set(HSLKERNEL_LIB_NAME libhsl_kernel )
  
else()
  # Name of C++ library
  set(LIBHSL_LIB_NAME hsl)
  
  # Name of C library
  set(LIBHSL_C_LIB_NAME hsl_c)

  # Name of Chipper library
  set(HSLCHIP_LIB_NAME hsl_chipper )  

  # Name of Kernel libray
  set(HSLKERNEL_LIB_NAME hsl_kernel )
endif()

# Name of test suite runner
set(LIBHSL_UNIT_TEST libhsl_test)

set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

# Choose package components
set(WITH_UTILITIES TRUE CACHE BOOL "Choose if libHSL utilities should be built")
set(WITH_TESTS TRUE CACHE BOOL "Choose if libHSL unit tests should be built")

# Choose to use pkg-config or not
set(WITH_PKGCONFIG FALSE CACHE BOOL
  "Choose whether a pkgconfig file (libHSL.pc) should be installed")

set(WITH_ENDIANAWARE FALSE CACHE BOOL "Choose whether or not libHSL should do runtime endianness switching.  Note that this can slow things down considerably if enabled by default")

# Enable CTest and submissions to libHSL dashboard at CDash
# http://my.cdash.org/index.php?project=libHSL
set(ENABLE_CTEST FALSE CACHE BOOL
  "Enable CTest to support submissions of results to CDash at http://cdash.org")

###############################################################################
# CMake settings
cmake_minimum_required(VERSION 2.6.0)

set(CMAKE_COLOR_MAKEFILE ON)

# Allow advanced users to generate Makefiles printing detailed commands
mark_as_advanced(CMAKE_VERBOSE_MAKEFILE)

# Path to additional CMake modules
set(CMAKE_MODULE_PATH ${libHSL_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

###############################################################################
# General build settings

# Default to debug mode until final 1.6.0 release 
if(NOT MSVC_IDE)
  if(NOT CMAKE_BUILD_TYPE)

  set(CMAKE_BUILD_TYPE Release CACHE STRING 
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel" FORCE)
  endif()
    message(STATUS "Setting libHSL build type - ${CMAKE_BUILD_TYPE}")
endif()

# TODO: Still testing the output paths --mloskot
set(LIBHSL_BUILD_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})

# Output directory in which to build RUNTIME target files.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LIBHSL_BUILD_OUTPUT_DIRECTORY})

# Output directory in which to build LIBRARY target files
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBHSL_BUILD_OUTPUT_DIRECTORY})

# Output directory in which to build ARCHIVE target files.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIBHSL_BUILD_OUTPUT_DIRECTORY}) 

file(READ "doc/index.txt" README )
file(WRITE "README.txt" "${README}")

###############################################################################
# Platform and compiler specific settings

if(WIN32)
  if (MSVC)
    if (MSVC80 OR MSVC90 OR MSVC10)
      add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
      add_definitions(-D_CRT_SECURE_NO_WARNINGS)
      add_definitions(-D_CRT_NONSTDC_NO_WARNING)
      add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    endif()
    
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
      string(REGEX REPLACE "/W[0-4]" "/W4"
        CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()

    # Generate dot-user file with user-specific settings for
    # Visual Studio project
    set(MSVC_ENVIRONMENT_PATH "" CACHE STRING
      "Custom PATH for Environment property in Visual Studio for project")
    mark_as_advanced(MSVC_ENVIRONMENT_PATH)
    set(VCPROJ_USER_ENVIRONMENT_PATH_DEBUG "${ENVIRONMENT_PATH}")
    mark_as_advanced(VCPROJ_USER_ENVIRONMENT_PATH_DEBUG)

  endif()
  
  set(CMAKE_INCLUDE_PATH c:/osgeo4w/include;$ENV{CMAKE_INCLUDE_PATH})
  set(CMAKE_LIBRARY_PATH c:/osgeo4w/lib;$ENV{CMAKE_LIBRARY_PATH})
else()

  # Recommended C++ compilation flags
  # -Weffc++
  set(LIBHSL_COMMON_CXX_FLAGS
	"-pedantic -ansi -Wall -Wpointer-arith -Wcast-align -Wcast-qual -Wfloat-equal -Wredundant-decls -Wno-long-long")

  if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LIBHSL_COMMON_CXX_FLAGS}")
    if (CMAKE_COMPILER_IS_GNUCXX)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()

  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR "${CMAKE_CXX_COMPILER}" MATCHES "clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LIBHSL_COMMON_CXX_FLAGS}")
  endif()
endif(WIN32)

if (APPLE)
  set(SO_EXT dylib)
  set(CMAKE_FIND_FRAMEWORK "LAST")
elseif(WIN32)
  set(SO_EXT dll)
else()
  set(SO_EXT so)
endif(APPLE)

###############################################################################
# Search for dependencies

# Boost C++ Libraries support - required
message(STATUS "Searching for Boost 1.38+ - done")

# Default to using static, multithreaded libraries for 
# linking under MSVC.  This is because we show users how to 
# use boostpro.com installer and install those options when linking 
# on windows in the compilation documentation.
if(WIN32)
  if (MSVC)
    set(Boost_USE_STATIC_LIBS   ON)
    set(Boost_USE_MULTITHREADED ON)
  endif(MSVC)
endif(WIN32)

# NOTE: Add iostreams to COMPONENTS list to enable bigfile_boost_iostreams_test
find_package(Boost 1.38 COMPONENTS program_options thread REQUIRED)

if(Boost_FOUND AND Boost_PROGRAM_OPTIONS_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# make these available for the user to set.
mark_as_advanced(CLEAR Boost_INCLUDE_DIR) 
mark_as_advanced(CLEAR Boost_LIBRARY_DIRS) 
link_directories(${Boost_LIBRARY_DIRS}) 

# GDAL/OGR support - optional, default=OFF
set(WITH_GDAL FALSE CACHE BOOL "Choose if GDAL support should be built")

if(WITH_GDAL)

  find_package(GDAL 1.7.0)
  include_directories(${GDAL_INCLUDE_DIR})
  add_definitions(-DHAVE_GDAL=1)

  mark_as_advanced(CLEAR GDAL_INCLUDE_DIR) 
  mark_as_advanced(CLEAR GDAL_LIBRARY) 

endif()

if(WITH_GDAL)
  if (NOT GEOTIFF_FOUND)
    message(FATAL_ERROR
      "GDAL support requires GeoTIFF library which was not selected")
  endif()
endif()


if(WITH_ENDIANAWARE)
    add_definitions(-DLIBHSL_ENDIAN_AWARE=1)
endif()
###############################################################################
# Installation settings

if(WIN32)
  set(DEFAULT_LIB_SUBDIR lib)
  set(DEFAULT_DATA_SUBDIR .)
  set(DEFAULT_INCLUDE_SUBDIR include)

  if (MSVC)
    set(DEFAULT_BIN_SUBDIR bin)
  else()
    set(DEFAULT_BIN_SUBDIR .)
  endif()
else()
  # Common locatoins for Unix and Mac OS X
  set(DEFAULT_BIN_SUBDIR bin)
  set(DEFAULT_LIB_SUBDIR lib)
  set(DEFAULT_DATA_SUBDIR share/libhsl)
  set(DEFAULT_INCLUDE_SUBDIR include)
endif()

# Locations are changeable by user to customize layout of libHSL installation
# (default values are platform-specific)
set(LIBHSL_BIN_SUBDIR ${DEFAULT_BIN_SUBDIR} CACHE STRING
  "Subdirectory where executables will be installed")
set(LIBHSL_LIB_SUBDIR ${DEFAULT_LIB_SUBDIR} CACHE STRING
  "Subdirectory where libraries will be installed")
set(LIBHSL_INCLUDE_SUBDIR ${DEFAULT_INCLUDE_SUBDIR} CACHE STRING
  "Subdirectory where header files will be installed")
set(LIBHSL_DATA_SUBDIR ${DEFAULT_DATA_SUBDIR} CACHE STRING
  "Subdirectory where data will be installed")

# Mark *_SUBDIR variables as advanced and dedicated to use by power-users only.
mark_as_advanced(LIBHSL_BIN_SUBDIR
  LIBHSL_LIB_SUBDIR LIBHSL_INCLUDE_SUBDIR LIBHSL_DATA_SUBDIR)

# Full paths for the installation
set(LIBHSL_BIN_DIR ${LIBHSL_BIN_SUBDIR})
set(LIBHSL_LIB_DIR ${LIBHSL_LIB_SUBDIR})
set(LIBHSL_INCLUDE_DIR ${LIBHSL_INCLUDE_SUBDIR})
set(LIBHSL_DATA_DIR ${LIBHSL_DATA_SUBDIR})

###############################################################################
# Installation commands

install(FILES AUTHORS COPYING INSTALL LICENSE README.md
  DESTINATION ${LIBHSL_DATA_DIR}/doc)

###############################################################################
# Processing of project directories

add_subdirectory(src)
add_subdirectory(samples)

if(WITH_UTILITIES)
  message(STATUS "Enable libHSL utilities to build - done")
  add_subdirectory(apps)
endif()

if(WITH_TESTS)
  message(STATUS "Enable libHSL unit tests to build - done")
  enable_testing()


  if(ENABLE_CTEST)
    message(STATUS
      "Enable CTest to support submissions of results to CDash at http://cdash.org")
    cmake_minimum_required(VERSION 2.8.0)
    # Dashboard has been prepared for experiments
    # http://my.cdash.org/index.php?project=libHSL
    include(CTest)
    message(STATUS
      "Enable CTest to support submissions of results to CDash at http://cdash.org - done")
  endif()

  add_subdirectory(test)
else()
  if(ENABLE_CTEST)
    message(WARNING
      "CTest support requested but WITH_TESTS option not specified to build of libHSL unit tests")
  endif()
endif()



# Version information
SET(CPACK_PACKAGE_VERSION_MAJOR ${LIBHSL_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${LIBHSL_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${LIBHSL_VERSION_MINOR})

SET(CPACK_SOURCE_GENERATOR "TGZ;ZIP;TBZ2")
SET(CPACK_CMAKE_GENERATOR "Unix Makefiles")



set(CPACK_SOURCE_PACKAGE_FILE_NAME
  "${CMAKE_PROJECT_NAME}-${VERSION}")
  
list(APPEND CPACK_SOURCE_IGNORE_FILES "/test/data/TO_core_last_zoom.hsp")
list(APPEND CPACK_SOURCE_IGNORE_FILES "_CPack_Packages")
list(APPEND CPACK_SOURCE_IGNORE_FILES ".gz")
list(APPEND CPACK_SOURCE_IGNORE_FILES ".bz2")
list(APPEND CPACK_SOURCE_IGNORE_FILES ".hg")
list(APPEND CPACK_SOURCE_IGNORE_FILES "README")
list(APPEND CPACK_SOURCE_IGNORE_FILES "HOWTORELEASE.txt")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/doc/_static/files")

list(APPEND CPACK_SOURCE_IGNORE_FILES "CMakeCache.txt")

list(APPEND CPACK_SOURCE_IGNORE_FILES "CPackConfig.cmake")
list(APPEND CPACK_SOURCE_IGNORE_FILES "schemas")
list(APPEND CPACK_SOURCE_IGNORE_FILES "hobu-config.bat")
list(APPEND CPACK_SOURCE_IGNORE_FILES "mpg-config.bat")

list(APPEND CPACK_SOURCE_IGNORE_FILES "libhsl-osgeo4w-init.bat")
list(APPEND CPACK_SOURCE_IGNORE_FILES "libhsl-osgeo4w-start.bat.tmpl")
list(APPEND CPACK_SOURCE_IGNORE_FILES "libhsl-osgeo4w-postinstall.bat")
include(CPack)

add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

#if (WIN32)
#    include(BuildOSGeo4W)
#endif(WIN32)
