#
# Build sample programs
#
SET( SAMPLE_READ sample_read )
SET( SAMPLE_WRITE sample_write )

INCLUDE_DIRECTORIES(
    ../include
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

link_directories (${LIBRARY_OUTPUT_PATH})
     
ADD_EXECUTABLE( ${SAMPLE_READ} read.cpp )
ADD_EXECUTABLE( ${SAMPLE_WRITE} write.cpp )

TARGET_LINK_LIBRARIES( ${SAMPLE_READ} PRIVATE Threads::Threads ${LIBHSL_LIB_NAME})
TARGET_LINK_LIBRARIES( ${SAMPLE_WRITE} PRIVATE Threads::Threads ${LIBHSL_LIB_NAME})

