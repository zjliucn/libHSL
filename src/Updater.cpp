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


#include "Updater.h"
#include <cfloat>

namespace hsl
{

Updater::Updater() : FileIO()
{

}

Updater::Updater(std::string filename) : FileIO(filename), _needHeaderCheck(false), _size(0),
_point(PointPtr(new Point(&DefaultHeader::get()))), _current(0), _filters(0), _transforms(0), _recordSize(0)
{

}

Updater::~Updater()
{

}

bool Updater::open()
{
	if (_filename == "")
		return false;

	_fp = fopen(_filename.c_str(), "rb+");
	if (_fp == nullptr)
		return false;

    if (!loadHeader())
        return false;

    _point->setHeader(_header.get());
    reset();

    return true;
}

void Updater::close()
{
    fclose(_fp);
}


void Updater::reset()
{
    fseek(_fp, 0, SEEK_SET);

    // Reset sizes and set internal cursor to the beginning of file.
    _current = 0;
    _size = _header->getPointRecordsCount();

    _recordSize = _header->getSchema().getByteSize();
}

Point & Updater::getPoint() const
{
    return *_point;
}

bool Updater::readNextPoint(bool readWaveform)
{
    if (_current == 0)
    {
        fseek(_fp, _header->getDataOffset(), SEEK_SET);
    }

    if (_current >= _size) {
        return false;
    }

    if (_needHeaderCheck)
    {
        if (!(_point->getHeader() == _header.get()))
            _point->setHeader(_header.get());
    }

    try
    {
        size_t count = fread(&_point->getData()[0], _recordSize, 1, _fp);
        if (count != 1)
            return false;

        ++_current;

    }
    catch (std::runtime_error&)
    {
        // If the stream is no good anymore, we're done reading points
        return false;
    }

    // Filter the points and continue reading until we either find 
    // one to keep or throw an exception.

    bool bLastPoint = false;

    if (!_filters.empty())
    {
        if (!filterPoint(*_point))
        {

            try
            {
                fread(&_point->getData().front(), _recordSize, 1, _fp);
                ++_current;

            }
            catch (std::runtime_error&)
            {
                // If the stream is no good anymore, we're done reading points
                return false;
            }

            while (!filterPoint(*_point))
            {

                try
                {
                    fread(&_point->getData().front(), _recordSize, 1, _fp);
                    ++_current;

                }
                catch (std::runtime_error&)
                {
                    // If the stream is no good anymore, we're done reading points
                    return false;
                }
                if (_current == _size)
                {
                    bLastPoint = true;
                    break;
                }
            }
        }
    }


    if (!_transforms.empty())
    {
        transformPoint(*_point);
    }

    if (bLastPoint)
        return false;

    if (readWaveform)
    {
        if (!readWaveformData())
            return false;
    }

    return true;
}

Point & Updater::readPointAt(std::size_t n, bool readWaveform)
{
    if (_size == n) {
        throw std::out_of_range("file has no more points to read, end of file reached");
    }
    else if (_size < n) {
        std::ostringstream msg;
        msg << "ReadPointAt:: Inputted value: " << n << " is greater than the number of points: " << _size;
        throw std::runtime_error(msg.str());
    }

    long pos = n * _header->getDataRecordLength() + _header->getDataOffset();

    fseek(_fp, pos, SEEK_SET);

    if (_needHeaderCheck)
    {
        if (!(_point->getHeader() == _header.get()))
            _point->setHeader(_header.get());
    }

    fread(&_point->getData().front(), _recordSize, 1, _fp);

    if (!_transforms.empty())
    {
        transformPoint(*_point);
    }

    if (readWaveform)
    {
        if (!readWaveformData())
            throw std::out_of_range("file has no more waveform data to read, end of file reached");
    }

    return *_point;
}

bool Updater::readWaveformData()
{
    try
    {
        uint64_t pos = 0;
        _point->getWaveformDataByteOffset(pos);
        uint32_t size = 0;
        _point->getWaveformDataSize(size);
        if (_point->isValid() && size > 0)
        {
            long pre = ftell(_fp);
            fseek(_fp, pos, SEEK_SET);
            _point->getWaveformData().resize(size);
            fread(&_point->getWaveformData().front(), size, 1, _fp);
            fseek(_fp, pre, SEEK_SET);    // back to previous position 
            return true;
        }
    }
    catch (std::runtime_error&)
    {
        // If the stream is no good anymore, we're done reading waveform data
        return false;
    }

    return true;
}

bool Updater::seek(size_t n)
{
    if (_size == n) {
        throw std::out_of_range("file has no more points to read, end of file reached");
        return false;
    }
    else if (_size < n) {
        std::ostringstream msg;
        msg << "Seek:: Inputted value: " << n << " is greater than the number of points: " << _size;
        throw std::runtime_error(msg.str());
        return false;
    }

    long pos = n * _header->getDataRecordLength() + _header->getDataOffset();
    fseek(_fp, pos, SEEK_SET);
    _current = n;

    return true;
}

size_t Updater::currentIndex()
{
    return _current;
}

bool Updater::writePoint(Point& point, bool updateWaveform)
{
    uint64_t offset = 0;
    uint32_t size = 0;
    // estimate and set waveform data address and size
    if (updateWaveform && getHeader().hasWaveformData())
    {
        if (point.hasWaveformData())
        {
            // Warning: the waveform data offset and size should keep no changes, otherwise file data will be corrupted
            point.getWaveformDataAddress(offset, size);
        }
    }

    std::vector<uint8_t> const& data = point.getData();
    fwrite(&data.front(), _header->getDataRecordLength(), 1, _fp);

    if (updateWaveform && getHeader().hasWaveformData())
    {
        if (point.hasWaveformData())
        {
            std::vector<uint8_t> const& waveformData = point.getWaveformData();
            if (getHeader().isInternalWaveformData())
            {
                // write waveform data to the file
                long pre = ftell(_fp);
                fseek(_fp, offset, SEEK_SET);
                fwrite(&waveformData.front(), size, 1, _fp);
                fseek(_fp, pre, SEEK_SET);    // back to previous position     
            }
            else
            {
                // write to an ancillary *.hsw waveform data file
                //TODO: save to hsw file
            }
        }
    }

    _current++;

    return true;
}

bool Updater::writePoint(const Point& point, bool updateWaveform)
{
    Point pt = point;
    return writePoint(pt, updateWaveform);
}

// update the field value by id for the current point record
bool Updater::writeFieldValuesById(FieldId id, const VariantArray& values)
{
    FieldArray d;
    const Schema& schema = _header->getSchema();
    bool found = schema.getFieldsById(id, d);
    if (!found || values.size() != d.size())
    {
        return false;
    }

    double scale = 1.0;
    double offset = 0.0;
    bool state = false;

    for (size_t i = 0; i < d.size(); i++)
    {
        const Variant& value = values[i];
        Field& field = d[i];
        if (field.isScaled() || field.isOffseted())
        {
            if (field.isScaled())
                scale = field.getScale();
            double offset = 0.0;
            if (field.isOffseted())
                offset = field.getOffset();

            Variant rawValue;
            if (getScaledValue(value, field.getDataType(), rawValue, scale, offset))
                state = writeRawValueToField(d[i], rawValue);
            else
                state = false;
        }
        else
        {
            Variant rawValue = value;
            state = writeRawValueToField(d[i], rawValue);
        }
    }

    return state;
}

// update the field value by field index for the current point record
bool Updater::writeFieldValue(size_t index, Variant& value)
{
    Field field;
    bool state = false;
    bool found = _header->getSchema().getField(index, field);
    if (!found)
    {
        return false;
    }

    double scale = 1.0;
    double offset = 0.0;
    if (field.isScaled() || field.isOffseted())
    {
        if (field.isScaled())
            scale = field.getScale();
        double offset = 0.0;
        if (field.isOffseted())
            offset = field.getOffset();
        Variant rawValue;
        if (getScaledValue(value, field.getDataType(), rawValue, scale, offset))
            state = writeRawValueToField(field, rawValue);
        else
            state = false;
    }
    else
        state = writeRawValueToField(field, value);

    return state;
}


bool Updater::writeRawValueToField(const Field& field, const Variant& value)
{
    size_t offset, size;
    bool state = false;

    long pos = _current * _header->getDataRecordLength() + _header->getDataOffset();
    if (fseek(_fp, pos, SEEK_SET) != 0)
        return false;

    switch (field.getDataType())
    {
    case DT_BIT:
    {
        boost::dynamic_bitset<> bitset(field.getBitSize());
        if (!value.getValue(bitset))
            state = false;
        offset = field.getByteOffset();
        size = field.getByteSize();
        size_t bitOffset = field.getBitOffset();
        size_t sizeInBits = field.getBitSize();
        if (sizeInBits < bitset.size())
            state = false;

        bool firstByte = true;
        size_t startBitPosition, stopBitPosition;
        size_t currentBit = 0;
        for (size_t i = 0; i < size; i++)
        {
            if (firstByte)
            {
                startBitPosition = bitOffset - sizeInBits % 8;
                if ((sizeInBits + startBitPosition) <= 8)
                    stopBitPosition = sizeInBits + startBitPosition - 1;
                else
                    stopBitPosition = 7;
                firstByte = false;
            }
            else if (i == size - 1) // last byte
            {
                startBitPosition = 0;
                stopBitPosition = bitOffset;
            }
            else
            {
                startBitPosition = 0;
                stopBitPosition = 7;
            }

            uint8_t v = 0;
            if (fseek(_fp, pos + offset + i, SEEK_SET) != 0 || fread(&v, sizeof(uint8_t), 1, _fp) != 1)
                return false;

            // Store value in bits 7
            uint8_t mask = 0;
            uint8_t bv = 0;
            for (size_t j = startBitPosition; j <= stopBitPosition; j++)
            {
                mask |= 0x1 << j;
                bv |= bitset[currentBit] << j;
                currentBit++;
            }
            v &= ~mask;
            v |= mask & bv;
            if (fseek(_fp, -sizeof(uint8_t), SEEK_CUR) != 0 || fwrite(&v, sizeof(uint8_t), 1, _fp) != 1)
                return false;;
        }

        state = true;
    }
    break;
    case DT_CHAR:
    {
        offset = field.getByteOffset();
        size = field.getByteSize();
        size_t s;
        if (value.getByteSize(s) && s <= size)
        {
            char *p_data = new char[size];
            if (p_data != nullptr)
            {
                if (value.getValue(p_data))
                {
                    if(fseek(_fp, pos + offset, SEEK_SET) == 0 && fwrite(p_data, size, 1, _fp) == 1)
                        state = true;
                }
                delete [] p_data;
            }
        }
    }
    break;
    case DT_UCHAR:
    {
        offset = field.getByteOffset();
        size = field.getByteSize();
        uint8_t v;
        if (value.getValue(v))
        {
            if (fseek(_fp, pos + offset, SEEK_SET) == 0 && fwrite(&v, sizeof(uint8_t), 1, _fp) == 1)
                state = true;
        }
    }
    break;
    case DT_SHORT:
    {
        offset = field.getByteOffset();
        size = field.getByteSize();
        int16_t v;
        if (value.getValue(v))
        {
            if (fseek(_fp, pos + offset, SEEK_SET) == 0 && fwrite(&v, sizeof(int16_t), 1, _fp) == 1)
                state = true;
        }
    }
    break;
    case DT_USHORT:
    {
        offset = field.getByteOffset();
        size = field.getByteSize();
        uint16_t v;
        if (value.getValue(v))
        {
            if (fseek(_fp, pos + offset, SEEK_SET) == 0 && fwrite(&v, sizeof(uint16_t), 1, _fp) == 1)
                state = true;
        }
    }
    break;
    case DT_LONG:
    {
        offset = field.getByteOffset();
        size = field.getByteSize();
        int32_t v;
        if (value.getValue(v))
        {
            if (fseek(_fp, pos + offset, SEEK_SET) == 0 && fwrite(&v, sizeof(int32_t), 1, _fp) == 1)
                state = true;
        }
    }
    break;
    case DT_ULONG:
    {
        offset = field.getByteOffset();
        size = field.getByteSize();
        uint32_t v;
        if (value.getValue(v))
        {
            if (fseek(_fp, pos + offset, SEEK_SET) == 0 && fwrite(&v, sizeof(uint32_t), 1, _fp) == 1)
                state = true;
        }
    }
    break;
    case DT_LONGLONG:
    {
        offset = field.getByteOffset();
        size = field.getByteSize();
        int64_t v;
        if (value.getValue(v))
        {
            if (fseek(_fp, pos + offset, SEEK_SET) == 0 && fwrite(&v, sizeof(int64_t), 1, _fp) == 1)
                state = true;
        }
    }
    break;
    case DT_ULONGLONG:
    {
        offset = field.getByteOffset();
        size = field.getByteSize();
        uint64_t v;
        if (value.getValue(v))
        {
            if (fseek(_fp, pos + offset, SEEK_SET) == 0 && fwrite(&v, sizeof(uint64_t), 1, _fp) == 1)
                state = true;
        }
    }
    break;
    case DT_FLOAT:
    {
        offset = field.getByteOffset();
        size = field.getByteSize();
        float v;
        if (value.getValue(v))
        {
            if (fseek(_fp, pos + offset, SEEK_SET) == 0 && fwrite(&v, sizeof(float), 1, _fp) == 1)
                state = true;
        }
    }
    break;
    case DT_DOUBLE:
    {
        offset = field.getByteOffset();
        size = field.getByteSize();
        double v;
        if (value.getValue(v))
        {
            if (fseek(_fp, pos + offset, SEEK_SET) == 0 && fwrite(&v, sizeof(double), 1, _fp) == 1)
                state = true;
        }
    }
    break;
    default:
        break;
    }

    fseek(_fp, pos, SEEK_SET);      // return to the start address of the point
    return state;
}


void Updater::transformPoint(hsl::Point& p)
{

    // Apply the transforms to each point
    std::vector<hsl::TransformPtr>::const_iterator ti;

    for (ti = _transforms.begin(); ti != _transforms.end(); ++ti)
    {
        hsl::TransformPtr transform = *ti;
        transform->transform(p);
    }
}

bool Updater::filterPoint(hsl::Point const& p)
{
    // If there's no filters on this reader, we keep 
    // the point no matter what.
    if (_filters.empty()) {
        return true;
    }

    std::vector<hsl::FilterPtr>::const_iterator fi;
    for (fi = _filters.begin(); fi != _filters.end(); ++fi)
    {
        hsl::FilterPtr filter = *fi;
        if (!filter->filter(p))
        {
            return false;
        }
    }
    return true;
}

void Updater::setFilters(std::vector<hsl::FilterPtr> const& filters)
{
    _filters = filters;
}

std::vector<hsl::FilterPtr> Updater::getFilters() const
{
    return _filters;
}

void Updater::setTransforms(std::vector<hsl::TransformPtr> const& transforms)
{
    _transforms = transforms;

    // Transforms are allowed to change the point, including moving the 
    // point's HeaderPtr.  We need to check if we need to set that 
    // back on any subsequent reads.
    if (_transforms.size() > 0)
    {
        for (std::vector<hsl::TransformPtr>::const_iterator i = transforms.begin(); i != transforms.end(); i++)
        {
            if (i->get()->ModifiesHeader())
                _needHeaderCheck = true;
        }
    }
}

std::vector<hsl::TransformPtr>  Updater::getTransforms() const
{
    return _transforms;
}


}