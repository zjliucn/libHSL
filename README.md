# libHSL
libHSL is a hyperspectral laser point cloud data format C/C++ development library. Using this library, user can easily read, write and update HSP hyperspectral laser point cloud data file format. By learning the advantages of LAS format specification, HSP file format supports flexible 3D point data structure configuration, including user-defined point attributes, multi band values and multi band waveform records, etc.

# libHSL interface
libHSL provides a development interface similar to libLAS (https://liblas.org/) library.

In order to migrate from ASPRS LAS data format to HSP data format, libHSL supports 9 predefined point cloud formats, including:

PF_ Pointformat0 defines the most basic XYZ format;

PF_ Pointformat1 defines the corresponding point data format 0 in Las point cloud format;

PF_ Pointformat2 defines the corresponding point data formats 1 and 6 in Las point cloud format;

PF_ Pointformat3 defines the corresponding point data format 2 in Las point cloud format;

PF_ Pointformat4 defines the corresponding point data formats 3 and 7 in Las point cloud format;

PF_ Pointformat5 defines the corresponding point data formats 4 and 9 in Las point cloud format;

PF_ Pointformat6 defines the corresponding point data format 5 in Las point cloud format;

PF_ Pointformat7 defines the corresponding point data format 8 in Las point cloud format;

PF_ Pointformat8 defines the corresponding point data format 10 in Las point cloud format.


# Tutorial
To learn how to use libHSL, please find the sample usage code from the sample directory. The sample directory contains sample code for reading and writing libHSL for reference.

# License
libHSL is available under the terms of the BSD License. See License for a copy.
