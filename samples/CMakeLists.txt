#
# Build sample programs
#
SET( SAMPLE_READ sample_read )
SET( SAMPLE_WRITE sample_write )
SET( SAMPLE_UPDATE sample_update )

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

INCLUDE_DIRECTORIES(
    ../include
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

link_directories (${LIBRARY_OUTPUT_PATH})


ADD_EXECUTABLE( ${SAMPLE_READ} read.cpp )
ADD_EXECUTABLE( ${SAMPLE_WRITE} write.cpp )
ADD_EXECUTABLE( ${SAMPLE_UPDATE} update.cpp )

TARGET_LINK_LIBRARIES( ${SAMPLE_READ} PRIVATE Threads::Threads ${Boost_LIBS} hsl)
TARGET_LINK_LIBRARIES( ${SAMPLE_WRITE} PRIVATE Threads::Threads ${Boost_LIBS} hsl)
TARGET_LINK_LIBRARIES( ${SAMPLE_UPDATE} PRIVATE Threads::Threads ${Boost_LIBS} hsl)

