/*************************************************************************************
 * 
 * 
 * Copyright (c) 2021, Zhengjun Liu <zjliu@casm.ac.cn>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 ************************************************************************************/


#pragma once

#include <stdint.h>
#include <memory>
#include <bitset>
#include <vector>
#include <boost/any.hpp>
#include <boost/array.hpp>
#include "hslLIB.h"
#include "IdDefinitions.h"


namespace hsl
{

const size_t FIELD_NAME_LENGTH = 32;
const size_t FIELD_DESCRIPTION_LENGTH = 32;

enum FormatVersion
{
    FV_VersionMajorMin = 1, ///< Minimum of major component
    FV_VersionMajorMax = 1, ///< Maximum of major component
    FV_VersionMinorMin = 0, ///< Minimum of minor component
    FV_VersionMinorMax = 0  ///< Maximum of minor component
};

enum DataType
{
  DT_BIT,
  DT_UCHAR,
  DT_CHAR,
  DT_USHORT,
  DT_SHORT,
  DT_ULONG,
  DT_LONG,
  DT_ULONGLONG,
  DT_LONGLONG,
  DT_FLOAT,
  DT_DOUBLE,
  DT_RESERVED,
  DT_UNKNOWN = 10000
};

#pragma pack(1)
class FileHeader
{
public:
  char          fileSignature[5];
  unsigned char majorVersion;
  unsigned char minorVersion;
  unsigned char byteOrder;
  uint64_t      numberOfPointRecords;
  uint64_t      pointDataOffset;  
  double        xMin;
  double        xMax;
  double        yMin;
  double        yMax;
  double        zMin;
  double        zMax;
  char          reserved[32];
  uint32_t      numberOfReturns;
};

class BlockOptionBits
{
public:
  unsigned char band_data_internal_bit : 1;
  unsigned char waveform_data_internal_bit : 1;
};

union BlockOptions
{
    BlockOptionBits bits;
    unsigned char c;
};

class BlockDesc
{
public:  
  unsigned long   fieldCount;
  unsigned long   fieldDefEncoding;
  unsigned short  characterEncoding;  // field name character encoding according to ANSI code page
  unsigned short  numberOfWaveformPacketDesc;
  BlockOptions    options;            // decide if waveform data is included in file from the lowest bit
  unsigned char   reserved[35];
};

class FieldDefinitionBits
{
public:
	unsigned char no_data_bit : 1;
	unsigned char min_bit : 1;
	unsigned char max_bit : 1;
	unsigned char scale_bit : 1;
	unsigned char offset_bit : 1;
	unsigned char size_in_bits_bit : 1;
	unsigned char reserved : 2;
};

union FieldDefinitionOptions
{
	FieldDefinitionBits bits;
	unsigned char c;
};

template <typename T>
class FieldDefinition
{
public:
	unsigned char           data_type;
	FieldDefinitionOptions  options;
	unsigned char           reserved[2];
	char                    name[FIELD_NAME_LENGTH];
	unsigned long           size_in_bits;
	T                       no_data;
	T                       min;
	T                       max;
	double                  scale;
	double                  offset;
	char                    description[FIELD_DESCRIPTION_LENGTH];
};

typedef bool bit;
template class FieldDefinition < bit > ;
template class FieldDefinition < char > ;
template class FieldDefinition < unsigned char > ;
template class FieldDefinition < short > ;
template class FieldDefinition < unsigned short > ;
template class FieldDefinition < long > ;
template class FieldDefinition < unsigned long > ;
template class FieldDefinition < long long > ;
template class FieldDefinition < unsigned long long > ;
template class FieldDefinition < float > ;
template class FieldDefinition < double > ;

typedef FieldDefinition<bit> BitField;
typedef FieldDefinition<char> CharField;
typedef FieldDefinition<unsigned char> UCharField;
typedef FieldDefinition<short> ShortField;
typedef FieldDefinition<unsigned short> UShortField;
typedef FieldDefinition<long> LongField;
typedef FieldDefinition<unsigned long> ULongField;
typedef FieldDefinition<long long> LongLongField;
typedef FieldDefinition<unsigned long long> ULongLongField;
typedef FieldDefinition<float> FloatField;
typedef FieldDefinition<double> DoubleField;

struct FieldDesc
{
public:
	FieldDesc() : type(DT_CHAR), data(Data()){}
	DataType  type;
	union Data
	{
		BitField        bitField;
		CharField       charField;
		UCharField      ucharField;
		ShortField      shortField;
		UShortField     ushortField;
		LongField       longField;
		ULongField      ulongField;
		LongLongField   longlongField;
		ULongLongField  ulonglongField;
		FloatField      floatField;
		DoubleField     doubleField;
	} data;
};

class WaveformPacketDesc
{
public:
	WaveformPacketDesc() {};
	WaveformPacketDesc(unsigned short theId, unsigned char theSampleBits, unsigned char theCompressType,
		unsigned long theSamples, unsigned long theInterval, double theGain, double theOffset)
		: id(theId), sampleBits(theSampleBits), compressType(theCompressType),
		samples(theSamples), interval(theInterval), gain(theGain), offset(theOffset){};
	bool operator==(const WaveformPacketDesc& other) const
	{
		if (&other == this) return true;
		if (memcmp(this, &other, sizeof(WaveformPacketDesc)) != 0) return false;
		return true;
	}
public:
	unsigned short    id;
	unsigned char     reserved[4];
	unsigned char     sampleBits;
	unsigned char     compressType;
	unsigned long     samples;
	unsigned long     interval;
	double            gain;
	double            offset;
};

class WaveformPacketDataDefinition
{
public:
	unsigned short   bandIndex;
	unsigned short   descriptorIndex;
	float            temporalOffset;
	float            dx;
	float            dy;
	float            dz;
	unsigned long    byteOffset;
	unsigned long    size;
};

#pragma pack()

typedef std::vector<WaveformPacketDesc> WaveformDesc;

#define RESERVED_BYTES_AFTER_FIELDS 128

/// Versions of point record format.
enum PointFormat
{
	PF_PointFormat0 = 0,  ///< Point Data Format \e 0
	PF_PointFormat1 = 1,  ///< Point Data Format \e 1
	PF_PointFormat2 = 2,  ///< Point Data Format \e 2
	PF_PointFormat3 = 3,  ///< Point Data Format \e 3
	PF_PointFormat4 = 4,  ///< Point Data Format \e 4
	PF_PointFormat5 = 5,  ///< Point Data Format \e 5
	PF_PointFormat6 = 6,  ///< Point Data Format \e 6    
	PF_PointFormat7 = 7,  ///< Point Data Format \e 7  
	PF_PointFormat8 = 8,  ///< Point Data Format \e 8  
	PF_PointFormatCustom = -1,  ///< Costum Point Data Format
	PF_PointFormatNone = -99,  ///< Costum Point Data Format
	PF_PointFormatUnknown = -100 ///< Point Data Format is unknown
};

}

