/******************************************************************************
 * Copyright (c) 2021, Zhengjun Liu, Howard Butler
 *
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following 
 * conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided 
 *       with the distribution.
 *     * Neither the name of the Martin Isenburg or Iowa Department 
 *       of Natural Resources nor the names of its contributors may be 
 *       used to endorse or promote products derived from this software 
 *       without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 ****************************************************************************/

#include "Header.h"
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstring>
#include <cassert>
#include <ctime>
#include "SpatialReference.h"
#include "Schema.h"
#include "Exception.h"
#include "detail/private_utility.hpp"


namespace hsl {

const size_t FieldDefinitionBasicSize = 88;
const unsigned short Header::_FileSignatureSize = 5;
const std::string Header::_FileSignature = "HSPCD";

Header::Header() : _schema(PF_PointFormatNone), _fileHeader(nullptr), _blockDesc(nullptr), _waveformDesc(nullptr),
    _pointRecordsByReturn(std::vector<uint64_t>()), _isCompressed(false)
{
    init();
}

Header::Header(PointFormat format) : _schema(format), _fileHeader(nullptr), _blockDesc(nullptr), _waveformDesc(nullptr),
    _pointRecordsByReturn(std::vector<uint64_t>()), _isCompressed(false)
{
    init();
}

Header::Header(const Schema &schema) : _schema(schema), _fileHeader(nullptr), _blockDesc(nullptr), _waveformDesc(nullptr),
    _pointRecordsByReturn(std::vector<uint64_t>()), _isCompressed(false)
{
    init();
}

Header::Header(Header const& other) : _schema(other._schema), _fileHeader(other._fileHeader), _blockDesc(other._blockDesc), 
    _waveformDesc(other._waveformDesc), _pointRecordsByReturn(other._pointRecordsByReturn), _isCompressed(other._isCompressed)
{
}

Header& Header::operator=(Header const& rhs)
{
    if (&rhs != this)
    {
    }
    return *this;
}

bool Header::operator==(Header const& other) const
{
    if (&other == this) return true;

    return true;
}

bool Header::isCompatible(const Header& other) const
{
    if (getReturnCount() != other.getReturnCount())
        return false;

    if (_schema != other.getSchema())
        return false;

    if (_waveformDesc->size() != other.getWaveformDesc()->size())
        return false;

    return true;
}

void Header::updateHeader()
{
	// update field count
	_blockDesc->fieldCount = _schema.getFieldCount();
	size_t size = calculateHeaderSize();
	setDataOffset(size);
}

std::string Header::getFileSignature() const
{
    return std::string(_fileHeader->fileSignature, _FileSignatureSize);
}

void Header::setFileSignature(std::string const& v)
{
    if (v.compare(0, _FileSignatureSize, Header::_FileSignature) != 0)
        throw std::invalid_argument("invalid file signature");

    std::strncpy(_fileHeader->fileSignature, v.c_str(), _FileSignatureSize);
}

uint8_t Header::getVersionMajor() const
{
    return _fileHeader->majorVersion;
}

void Header::setVersionMajor(uint8_t v)
{
    if (FV_VersionMajorMin > v || v > FV_VersionMajorMax)
        throw std::out_of_range("version major out of range");

    _fileHeader->majorVersion = v;
}

uint8_t Header::getVersionMinor() const
{
    return _fileHeader->minorVersion;
}

void Header::setVersionMinor(uint8_t v)
{
    if (v > FV_VersionMinorMax)
        throw std::out_of_range("version minor out of range");

    _fileHeader->minorVersion = v;
}

uint32_t Header::getDataOffset() const
{
    return _fileHeader->pointDataOffset;
}

void Header::setDataOffset(uint32_t v)
{
    _fileHeader->pointDataOffset = v;
}

void Header::setDataFormat(PointFormat v)
{
    _schema.setDataFormat(v);
	//update block descriptor and field descriptor
	_blockDesc->fieldCount = _schema.getFieldCount();
}

uint32_t Header::getDataRecordLength() const
{
    // No matter what the schema says, this must be a a short in size.
    return static_cast<uint32_t>(_schema.getByteSize());
}

uint64_t Header::getPointRecordsCount() const
{
    return _fileHeader->numberOfPointRecords;
}

void Header::setPointRecordsCount(uint64_t v)
{
    _fileHeader->numberOfPointRecords = v;
}

uint32_t Header::getReturnCount() const
{
    return _fileHeader->numberOfReturns;
}

void Header::setReturnCount(uint32_t v)
{
    _fileHeader->numberOfReturns = v;
    _pointRecordsByReturn.resize(v);
}

bool Header::setPointRecordByReturn(std::size_t index, uint64_t v)
{
    if (index >= _pointRecordsByReturn.size())
        return false;

    uint64_t& t = _pointRecordsByReturn.at(index);
    t = v;

    return true;
}

uint64_t Header::getPointRecordByReturn(std::size_t index) const
{
    return _pointRecordsByReturn[index];    
}

std::vector<uint64_t> Header::getPointRecordsByReturnCount() const 
{
    return _pointRecordsByReturn; 
}


double Header::getScaleX() const
{
    FieldArray fa;
    _schema.getFieldsById(FI_X, fa);
    return fa[0].getScale();
}

double Header::getScaleY() const
{
    FieldArray fa;
    _schema.getFieldsById(FI_Y, fa);
    return fa[0].getScale();
}

double Header::getScaleZ() const
{
    FieldArray fa;
    _schema.getFieldsById(FI_Z, fa);
    return fa[0].getScale();
}

void Header::setScale(double x, double y, double z)
{
    Field* field = _schema.getFieldById(FI_X);
	if (field != NULL)
	{
		if (field->isScaled() == false)
            field->isScaled(true);
        field->setScale(x);
	}

	field = _schema.getFieldById(FI_Y);
	if (field != NULL)
	{
		if (field->isScaled() == false)
            field->isScaled(true);
        field->setScale(y);
	}

	field = _schema.getFieldById(FI_Z);
	if (field != NULL)
	{
		if (field->isScaled() == false)
            field->isScaled(true);
        field->setScale(z);
	}
}

double Header::getOffsetX() const
{
    FieldArray fa;
    _schema.getFieldsById(FI_X, fa);
    return fa[0].getOffset();
}

double Header::getOffsetY() const
{
    FieldArray fa;
    _schema.getFieldsById(FI_Y, fa);
    return fa[0].getOffset();
}

double Header::getOffsetZ() const
{
    FieldArray fa;
    _schema.getFieldsById(FI_Z, fa);
    return fa[0].getOffset();
}

void Header::setOffset(double x, double y, double z)
{
	Field* field = _schema.getFieldById(FI_X);
	if (field != NULL)
	{
		if (field->isOffseted() == false)
            field->isOffseted(true);
        field->setOffset(x);
	}

	field = _schema.getFieldById(FI_Y);
	if (field != NULL)
	{
		if (field->isOffseted() == false)
            field->isOffseted(true);
        field->setOffset(y);
	}

	field = _schema.getFieldById(FI_Z);
	if (field != NULL)
	{
		if (field->isOffseted() == false)
            field->isOffseted(true);
        field->setOffset(z);
	}
}

double Header::getMaxX() const
{
    return _fileHeader->xMax;
}

double Header::getMinX() const
{
    return _fileHeader->xMin;
}

double Header::getMaxY() const
{
    return _fileHeader->yMax;
}

double Header::getMinY() const
{
    return _fileHeader->yMin;
}

double Header::getMaxZ() const
{
    return _fileHeader->zMax;
}

double Header::getMinZ() const
{
    return _fileHeader->zMin;
}

void Header::setMax(double x, double y, double z)
{
    _fileHeader->xMax = x;
    _fileHeader->yMax = y;
    _fileHeader->zMax = z;
}

void Header::setMin(double x, double y, double z)
{
    _fileHeader->xMin = x;
    _fileHeader->yMin = y;
    _fileHeader->zMin = z;
}

void Header::setExtent(Bounds<double> const& extent)
{
    _fileHeader->xMax = extent.max(0);
    _fileHeader->yMax = extent.max(1);
    _fileHeader->zMax = extent.max(2);
    _fileHeader->xMin = extent.min(0);
    _fileHeader->yMin = extent.min(1);
    _fileHeader->zMin = extent.min(2);
}

const Bounds<double>& Header::getExtent() const
{    
    return Bounds<double>(_fileHeader->xMin, _fileHeader->yMin, _fileHeader->zMin, _fileHeader->xMax, _fileHeader->yMax, _fileHeader->zMax);;
}

size_t Header::calculateHeaderSize()
{
    size_t size = sizeof(FileHeader) + _fileHeader->numberOfReturns * sizeof(uint64_t) + sizeof(BlockDesc);

    for (size_t i = 0; i < _blockDesc->fieldCount; i++)
    {
        size += sizeof(unsigned long); // count field id
        Field dim;
        _schema.getField(i, dim);
        // FieldDefinition size is variable according to data type for no_data, min, max
        size += dim.getByteSize() * 3 + FieldDefinitionBasicSize; 
    }

    size += _blockDesc->numberOfWaveformPacketDesc * sizeof(WaveformPacketDesc); // count waveform field bytes
    size += RESERVED_BYTES_AFTER_FIELDS; // count reserved bytes
    
    return size;
}

void Header::init()
{
    // Initialize  header with default values
    _fileHeader = std::make_shared<FileHeader>();
    _blockDesc = std::make_shared<BlockDesc>();
    _waveformDesc = std::make_shared<WaveformDesc>();

    _fileHeader->majorVersion = 1;
    _fileHeader->minorVersion = 0;
    _fileHeader->numberOfPointRecords = 0;
    _fileHeader->numberOfReturns = 0;

    _fileHeader->pointDataOffset = calculateHeaderSize();

    std::memset(_fileHeader->fileSignature, 0, sizeof(_FileSignature) - 1);
    std::strncpy(_fileHeader->fileSignature, _FileSignature.c_str(), _FileSignatureSize);

    _blockDesc->fieldCount = _schema.getFieldCount();

    _blockDesc->numberOfWaveformPacketDesc = 0;
    setInternalWaveformData(true);
    setInternalBandData(true);

    setScale(1.0, 1.0, 1.0);

    _isCompressed = false;
}

const Schema & Header::getSchema() const
{
    return _schema;
}

Schema & Header::getSchema()
{
    return _schema;
}

void Header::setSchema(const Schema& schema)
{
    _schema = schema;

    // Reset the X, Y, Z dimensions with offset and scale values
	Field* field = _schema.getFieldById(FI_X);
	if (field != NULL)
        throw libhsl_error("X dimension not on schema, you\'ve got big problems!");
    field->setScale(1.0);
    field->isFinitePrecision(true);
    field->setOffset(0.0);

	field = _schema.getFieldById(FI_Y);
	if (field != NULL)
        throw libhsl_error("Y dimension not on schema, you\'ve got big problems!");
    field->setScale(1.0);
    field->isFinitePrecision(true);
    field->setOffset(0.0);

	field = _schema.getFieldById(FI_Z);
	if (field != NULL)
        throw libhsl_error("Z dimension not on schema, you\'ve got big problems!");
    field->setScale(1.0);
    field->isFinitePrecision(true);
    field->setOffset(0.0);
}

void Header::setCompressed(bool b)
{
    _isCompressed = b;
}

bool Header::isCompressed() const
{
    return _isCompressed;
}

bool Header::hasWaveformData() const
{
	return (_blockDesc->numberOfWaveformPacketDesc > 0) && 
		_schema.hasField(FI_ByteOffsetToWaveformData) && 
		_schema.hasField(FI_WaveformDataSize);
}

bool Header::addWaveformPacketDesc(const WaveformPacketDesc &descriptor)
{
	_waveformDesc->push_back(descriptor);
	_blockDesc->numberOfWaveformPacketDesc++;
	return true;
}

bool Header::isInternalWaveformData() const
{
    return (_blockDesc->options.bits.waveform_data_internal_bit > 0);
}

void Header::setInternalWaveformData(bool b)
{
    _blockDesc->options.bits.waveform_data_internal_bit = 1;
}

bool Header::isInternalBandData() const
{
    return (_blockDesc->options.bits.band_data_internal_bit > 0);
}

void Header::setInternalBandData(bool b)
{
    _blockDesc->options.bits.band_data_internal_bit = 1;
}

}
