###############################################################################
# CMake configuration file for libHSL src
#
# Author: Zhengjun Liu <zjliu@casm.ac.cn>
#
#
###############################################################################

cmake_minimum_required (VERSION 3.0.0)
project (libHSL VERSION ${PROJECT_VERSION})

# set output directory
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(GDAL_FOUND)
    source_group("Source Files\\gdal" FILES ${LIBLAS_GDAL_CPP})
endif()

# Diable Visual C++ language extensions when building libHSL library
# Need this enabled for boost to work -- hobu
#if (WIN32)
#  if (MSVC)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Za")
#  endif()
#endif()

# Build the library...
include_directories (${Boost_INCLUDE_DIR})

# Standard include directory of libHSL library
include_directories(../include)
link_directories (${Boost_LIBRARY_DIR_RELEASE})

# Include all the .cpp files in the library.
file (GLOB SOURCES [A-Za-z]*.cpp index/[A-Za-z]*.cpp)
file (GLOB HEADERS [A-Za-z]*.h index/[A-Za-z]*.h)

# Define the library and specify whether it is shared or not.
if (LIBHSL_SHARED_LIB)
  if (WIN32)
    ADD_DEFINITIONS(-DLIBHSL_DLL_EXPORT)
  endif ()
  add_library (${LIBHSL_LIB_NAME} SHARED ${SOURCES} ${HEADERS})
else ()
  add_library (${LIBHSL_LIB_NAME} STATIC ${SOURCES} ${HEADERS})
endif ()

if (WIN32)
  target_link_libraries (${LIBHSL_LIB_NAME} "")
else ()
  target_link_libraries (${LIBHSL_LIB_NAME} pthread)
endif ()

# Set the version number on the library
if (WIN32)
  set_target_properties (${LIBHSL_LIB_NAME} PROPERTIES VERSION "${LIBRARY_VERSION}" )
else ()
  set_target_properties (${LIBHSL_LIB_NAME} PROPERTIES
    VERSION "${LIBRARY_VERSION}" SOVERSION "${LIBRARY_SOVERSION}" )
endif ()

# Specify where the library is installed, adding it to the export depends
install (TARGETS ${LIBHSL_LIB_NAME}
  EXPORT depends
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib${LIB_SUFFIX}
  ARCHIVE DESTINATION lib${LIB_SUFFIX})

# Put all the library into a folder in the IDE
set_property (TARGET ${LIBHSL_LIB_NAME} PROPERTY FOLDER library)
