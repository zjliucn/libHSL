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


#include "FileIO.h"
#include <cfloat>
#include <boost/dynamic_bitset.hpp>

namespace hsl
{


bool loadFieldDesc(FILE* fp, FieldDesc &fieldDesc);
bool saveFieldDesc(FILE* fp, const FieldDesc &fieldDesc);

FileIO::FileIO() : _filename(""), _fp(nullptr), _header(nullptr)
{
}

FileIO::FileIO(std::string filename) : _fp(nullptr), _header(nullptr)
{
	_filename = filename;
}

FileIO::~FileIO()
{

}

SpatialReference FileIO::getSRS() const
{
    return _srs;
}

void FileIO::setSRS(SpatialReference& srs)
{
    _srs = srs;
}

Header const& FileIO::getHeader() const 
{
	return *_header;
}

Header& FileIO::getHeader()
{
	return *_header;
}

void FileIO::setHeader(Header const& header)
{
	_header = HeaderPtr(new Header(header));
}


void FileIO::setFilename(std::string filename)
{
	_filename = filename;
}

bool FileIO::loadHeader()
{
	if (_header == nullptr)
    	_header = std::make_shared<Header>();
		
	if (_header == nullptr)
		return false;

	if (fread(_header->getFileHeader().get(), sizeof(FileHeader), 1, _fp) != 1)
		return false;

	unsigned long returnCount = _header->getFileHeader()->numberOfReturns;
	_header->setReturnCount(returnCount);
	for (size_t i = 0; i < returnCount; i++)
	{
		uint64_t pointCount;
		size_t count = fread(&pointCount, sizeof(uint64_t), 1, _fp);
		if (count == 1)
			_header->setPointRecordByReturn(i, pointCount);
		else
		{
			return false;
		}		
	}

	if (fread(_header->getBlockDesc().get(), sizeof(BlockDesc), 1, _fp) != 1)
		return false;

	// read fields accordingly
	size_t fieldCount = _header->getBlockDesc()->fieldCount;
	FieldDesc fieldDesc;
	for (size_t i = 0; i < fieldCount; i++)
	{
		 uint32_t fieldId;
		 size_t count = fread(&fieldId, sizeof(uint32_t), 1, _fp);
		if (count != 1)
		{			
			return false;
		}

		if (loadFieldDesc(_fp, fieldDesc))
			_header->getSchema().addFieldFromFieldDesc((FieldId)fieldId, fieldDesc);
	}

	// read waveform information
	if (_header->getBlockDesc()->numberOfWaveformPacketDesc != 0)
	{
		WaveformPacketDesc wd;
		for (auto i = 0; i < _header->getBlockDesc()->numberOfWaveformPacketDesc; i++)
		{
			size_t count = fread(&wd, sizeof(WaveformPacketDesc), 1, _fp);
			if (count != 1)
			{			
				return false;
			}

			_header->getWaveformDesc()->push_back(wd);
		}
	}	

	// skip reserved area
	int result = fseek(_fp, RESERVED_BYTES_AFTER_FIELDS, SEEK_CUR);
	if (result != 0)
		return false;

	return true;
}

bool FileIO::writeHeader()
{
	if (_header == nullptr)
		return false;

	_header->updateHeader();

	if (fwrite(_header->getFileHeader().get(), sizeof(FileHeader), 1, _fp) != 1)
		return false;

	unsigned long returnCount = _header->getReturnCount();
	for (size_t i = 0; i < returnCount; i++)
	{
		uint64_t pointCount = _header->getPointRecordByReturn(i);
		size_t count = fwrite(&pointCount, sizeof(uint64_t), 1, _fp);
		if (count != 1)
		{
			return false;
		}		
	}

	if (fwrite(_header->getBlockDesc().get(), sizeof(BlockDesc), 1, _fp) != 1)
		return false;

	// write schema as fields accordingly
	size_t fieldCount = _header->getBlockDesc()->fieldCount;
	FieldDesc fieldDesc;
	for (size_t i = 0; i < fieldCount; i++)
	{
		 Field field;
		 if(_header->getSchema().getField(i, field))
		 {
			 uint32_t id = (uint32_t)field.getId();
			 fwrite(&id, sizeof(uint32_t), 1, _fp);
			 Schema::getFieldDescFromField(field, fieldDesc);
			 if (!saveFieldDesc(_fp, fieldDesc))
				 return false;
		 }
	}

	// write waveform information
	if (_header->getBlockDesc()->numberOfWaveformPacketDesc != 0)
	{
		assert(_header->getBlockDesc()->numberOfWaveformPacketDesc == _header->getWaveformDesc()->size());
		WaveformPacketDesc wd;
		for (size_t i = 0; i < _header->getWaveformDesc()->size(); i++)
		{
			wd = (*_header->getWaveformDesc())[i];
			size_t count = fwrite(&wd, sizeof(WaveformPacketDesc), 1, _fp);
			if (count != 1)
			{			
				return false;
			}
		}
	}

	// write reserved area
	char reserved[RESERVED_BYTES_AFTER_FIELDS];
	std::memset(reserved, 0, RESERVED_BYTES_AFTER_FIELDS);
	size_t result = fwrite(reserved, sizeof(char), RESERVED_BYTES_AFTER_FIELDS, _fp);
	if (result != RESERVED_BYTES_AFTER_FIELDS)
		return false;

	return true;
}

bool FileIO::updateHeader(Header const& header)
{
	if (_header == nullptr || !_header->isCompatible(header))
		return false;

	setHeader(header);
	long pre = ftell(_fp);
	fseek(_fp, 0, SEEK_SET);
	bool state = writeHeader();
	fseek(_fp, pre, SEEK_SET);    // back to previous position

	return state;
}

bool loadFieldDesc(FILE* fp, FieldDesc &fieldDesc)
{
	unsigned char type;
	size_t result = fread(&type, sizeof(unsigned char), 1, fp);
	if (result == 1)
	{
		fseek(fp, -1, SEEK_CUR);
		fieldDesc.type = (DataType)type;
		size_t count;
		switch (fieldDesc.type)
		{
			case DT_BIT:
				count = fread(&fieldDesc.data.bitField, sizeof(BitField), 1, fp);
				if (count != 1)
					return false;
				break;
			case DT_CHAR:
				count = fread(&fieldDesc.data.charField, sizeof(CharField), 1, fp);
				if (count != 1)
					return false;				
				break;
			case DT_UCHAR:
				count = fread(&fieldDesc.data.ucharField, sizeof(UCharField), 1, fp);
				if (count != 1)
					return false;				
				break;
			case DT_SHORT:
				count = fread(&fieldDesc.data.shortField, sizeof(ShortField), 1, fp);
				if (count != 1)
					return false;				
				break;
			case DT_USHORT:
				count = fread(&fieldDesc.data.ushortField, sizeof(UShortField), 1, fp);
				if (count != 1)
					return false;				
				break;
			case DT_LONG:
				count = fread(&fieldDesc.data.longField, sizeof(LongField), 1, fp);
				if (count != 1)
					return false;				
				break;
			case DT_ULONG:
				count = fread(&fieldDesc.data.ulongField, sizeof(ULongField), 1, fp);
				if (count != 1)
					return false;				
				break;
			case DT_LONGLONG:
				count = fread(&fieldDesc.data.longlongField, sizeof(LongLongField), 1, fp);
				if (count != 1)
					return false;				
				break;
			case DT_ULONGLONG:
				count = fread(&fieldDesc.data.ulonglongField, sizeof(ULongLongField), 1, fp);
				if (count != 1)
					return false;				
				break;
			case DT_FLOAT:
				count = fread(&fieldDesc.data.floatField, sizeof(FloatField), 1, fp);
				if (count != 1)
					return false;				
				break;
			case DT_DOUBLE:
				count = fread(&fieldDesc.data.doubleField, sizeof(DoubleField), 1, fp);
				if (count != 1)
					return false;				
				break;
			default:
				return false;

		}
		return true;
	}
	else
	{
		return false;
	}
}

bool saveFieldDesc(FILE* fp, const FieldDesc &fieldDesc)
{
	size_t count;
	switch (fieldDesc.type)
	{
	case DT_BIT:
		count = fwrite(&fieldDesc.data.bitField, sizeof(BitField), 1, fp);
		if (count != 1)
			return false;
		break;
	case DT_CHAR:
		count = fwrite(&fieldDesc.data.charField, sizeof(CharField), 1, fp);
		if (count != 1)
			return false;
		break;
	case DT_UCHAR:
		count = fwrite(&fieldDesc.data.ucharField, sizeof(UCharField), 1, fp);
		if (count != 1)
			return false;
		break;
	case DT_SHORT:
		count = fwrite(&fieldDesc.data.shortField, sizeof(ShortField), 1, fp);
		if (count != 1)
			return false;
		break;
	case DT_USHORT:
		count = fwrite(&fieldDesc.data.ushortField, sizeof(UShortField), 1, fp);
		if (count != 1)
			return false;
		break;
	case DT_LONG:
		count = fwrite(&fieldDesc.data.longField, sizeof(LongField), 1, fp);
		if (count != 1)
			return false;
		break;
	case DT_ULONG:
		count = fwrite(&fieldDesc.data.ulongField, sizeof(ULongField), 1, fp);
		if (count != 1)
			return false;
		break;
	case DT_LONGLONG:
		count = fwrite(&fieldDesc.data.longlongField, sizeof(LongLongField), 1, fp);
		if (count != 1)
			return false;
		break;
	case DT_ULONGLONG:
		count = fwrite(&fieldDesc.data.ulonglongField, sizeof(ULongLongField), 1, fp);
		if (count != 1)
			return false;
		break;
	case DT_FLOAT:
		count = fwrite(&fieldDesc.data.floatField, sizeof(FloatField), 1, fp);
		if (count != 1)
			return false;
		break;
	case DT_DOUBLE:
		count = fwrite(&fieldDesc.data.doubleField, sizeof(DoubleField), 1, fp);
		if (count != 1)
			return false;
		break;
	default:
		return false;

	}
	return true;
}

}