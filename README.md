# libHSL
libHSL is a hyperspectral laser point cloud data format C/C++ development library. Using this library, user can easily read, write and update HSP hyperspectral laser point cloud data file format. By learning the advantages of LAS format specification, HSP file format supports flexible 3D point data structure configuration, including user-defined point attributes, multi band values and multi band waveform records, etc.

# Install
libHSL requires:
  - boost 1.38 or greater
  - cmake 2.6 or greater

For Unix/Linux/Mac:

    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=_preferred_path_ ..
    make
    make install
    /sbin/ldconfig #if necessary

# Tutorial
libHSL provides a development interface similar to libLAS (https://liblas.org/) library.

In order to migrate from ASPRS LAS data format to HSP data format, libHSL supports 9 predefined point cloud formats, including:
  - PF_ PointFormat0 defines the most basic XYZ format;
  - PF_ PointFormat1 defines the corresponding point data format 0 in Las point cloud format;
  - PF_ PointFormat2 defines the corresponding point data formats 1 and 6 in Las point cloud format;
  - PF_ PointFormat3 defines the corresponding point data format 2 in Las point cloud format;
  - PF_ PointFormat4 defines the corresponding point data formats 3 and 7 in Las point cloud format;
  - PF_ PointFormat5 defines the corresponding point data formats 4 and 9 in Las point cloud format;
  - PF_ PointFormat6 defines the corresponding point data format 5 in Las point cloud format;
  - PF_ PointFormat7 defines the corresponding point data format 8 in Las point cloud format;
  - PF_ PointFormat8 defines the corresponding point data format 10 in Las point cloud format.

To learn how to use libHSL, please find the sample usage code from the sample directory. The sample directory contains sample code for reading and writing libHSL for reference.

# Contribute

As a community developed and maintained repository, we welcome new quality contributions. The recommended way to submit your changes is via a pull request.

Before submitting a patch please ensure:

  * Patched code compiles.
  * The patch is fixing a specific issue or implementing a new feature
    (it’s not doing multiple things at the same time).
  * unit tests have been updated.
  * unit tests are passing.

# License
libHSL is available under the terms of the BSD License. See License for a copy.
