/******************************************************************************
 * Copyright (c) 2021, Zhengjun Liu <zjliu@casm.ac.cn>
 * Copyright (c) 2008, Mateusz Loskot <mateusz@loskot.net>
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

#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#include <iosfwd>
#include <algorithm>
#include <numeric>
#include <boost/dynamic_bitset.hpp>
#include <boost/lexical_cast.hpp>
#include "detail/private_utility.hpp"
#include "Point.h"
#include "Exception.h"
#include "Header.h"
#include "Field.h"


using namespace boost;


namespace hsl {


// descale the value given our scale/offset
template <typename T>
void getScaledValue(const T &valueIn, T &valueOut, double scale, double offset, ScaleOffsetOp scaleInOut)
{
    if (scaleInOut == SO_In)
    {
        valueOut = static_cast<T>(detail::sround((valueIn - offset) / scale));
    }else if (scaleInOut == SO_Out)
    {
        valueOut = static_cast<T>(detail::sround(scale * valueIn + offset));
    }
}

bool getScaledValue(const Variant &value, DataType rawType, Variant &rawValue, double scale, double offset)
{
    bool state = false;
    switch (rawType)
    {
        case DT_BIT:
        case DT_CHAR:
        case DT_UCHAR:
            {
                rawValue = value;
                state = true;                
            }
            break;
        case DT_SHORT:
            {
                double v;
                if(value.getValue(v))
                {
                    rawValue.setValue(static_cast<int16_t>(detail::sround((v - offset) / scale)));                
                    state = true;
                }
            }
            break;
        case DT_USHORT:
            {
                double v;
                if(value.getValue(v))
                {
                    rawValue.setValue(static_cast<uint16_t>(detail::sround((v - offset) / scale)));  
                    state = true;
                }              
            }
            break;
        case DT_LONG:
            {
                double v;
                if(value.getValue(v))
                {
                    rawValue.setValue(static_cast<int32_t>(detail::sround((v - offset) / scale)));
                    state = true;
                }                
            }
            break;
        case DT_ULONG:
            {
                double v;
                if(value.getValue(v))
                {
                    rawValue.setValue(static_cast<uint32_t>(detail::sround((v - offset) / scale))); 
                    state = true;
                }               
            }
            break;
        case DT_LONGLONG:
            {
                double v;
                if(value.getValue(v))
                {
                    rawValue.setValue(static_cast<int64_t>(detail::sround((v - offset) / scale))); 
                    state = true;
                }               
            }
            break;
        case DT_ULONGLONG:
            {
                double v;
                if (value.getValue(v))
                {
                    rawValue.setValue(static_cast<uint64_t>(detail::sround((v - offset) / scale))); 
                    state = true;
                }               
            }
            break;
        case DT_FLOAT:
            {
                double v;
                if(value.getValue(v))
                {
                    rawValue.setValue(static_cast<float>((v - offset) / scale));  
                    state = true;
                }              
            }
            break;            
        case DT_DOUBLE:
            {
                double v;
                if (value.getValue(v))
                {
                    rawValue.setValue((v - offset) / scale);                
                    state = true;
                }
            }
            break;
        default:
            {
                state = false;
            }
            break;
    }

    return state;
}


Point::Point(const Header * hdr)
    : 
     _header(hdr)
    , _default_header(DefaultHeader::get())
{
    _data.resize(hdr->getDataRecordLength());
    _data.assign(hdr->getDataRecordLength(), 0);
}

Point::Point(const Point & other)
    : _data(other._data)
    , _waveformData(other._waveformData)
    , _header(other.getHeader())
    , _default_header(DefaultHeader::get())
{
}

Point& Point::operator=(const Point & rhs)
{
    if (&rhs != this)
    {
        _data = rhs._data;
        _waveformData = rhs._waveformData;
        _header = rhs._header;
    }
    return *this;
}

void Point::setCoordinates(double const& x, double const& y, double const& z)
{
    setX(x);
    setY(y);
    setZ(z);
}

bool Point::equal(Point const& other) const
{
    // TODO - mloskot: Default epsilon is too small.
    //                 Is 0.00001 good as tolerance or too wide?
    //double const epsilon = std::numeric_limits<double>::epsilon(); 
    double const epsilon = 0.00001;

    double const dx = getX() - other.getX();
    double const dy = getY() - other.getY();
    double const dz = getZ() - other.getZ();

    // TODO: Should we compare other data members, besides the coordinates?

    if ((dx <= epsilon && dx >= -epsilon)
     && (dy <= epsilon && dy >= -epsilon)
     && (dz <= epsilon && dz >= -epsilon))
    {
        return true;
    }

    // If we do other members
    // bool compare_classification(uint8_t cls, uint8_t expected)
    // {
    //    // 31 is max index in classification lookup table
    //    clsidx = (cls & 31);
    //    assert(clsidx <= 31); 
    //    return (clsidx == expected);
    // }

    return false;
}

bool Point::validate() const
{
    unsigned int flags = 0;
    //TODO: validate point

    return true;
}

bool Point::isValid() const
{
    //TODO: test if it is valid
    return true;
}

void Point::setHeader(Header const* header)
{

    if (!header)
    {
        throw libhsl_error("header reference for setHeader is void");
    }
    
    // If we don't have a header initialized, set the point's to the 
    // one we were given.
    if (!_header) _header = header;

    // This is hopefully faster than copying everything if we don't have 
    // any data set and nothing to worry about.
    uint16_t wanted_length = header->getDataRecordLength();
    uint32_t sum = std::accumulate(_data.begin(), _data.end(), 0);
    
    if (!sum) {
        std::vector<uint8_t> data;
        data.resize(wanted_length);
        data.assign(wanted_length, 0);
        _data = data;
        _header = header;
        return;
    }
    
    bool bApplyNewScaling = true;
    
    if (detail::compare_distance(header->getScaleX(), _header->getScaleX()) &&
        detail::compare_distance(header->getScaleY(), _header->getScaleY()) &&
        detail::compare_distance(header->getScaleZ(), _header->getScaleZ()) &&
        detail::compare_distance(header->getOffsetX(), _header->getOffsetX()) &&
        detail::compare_distance(header->getOffsetY(), _header->getOffsetY()) &&
        detail::compare_distance(header->getOffsetZ(), _header->getOffsetZ()))
        bApplyNewScaling = false;
    else
        bApplyNewScaling = true;
    
    if (wanted_length != _data.size())
    {
        // Manually copy everything but the header ptr
        // We can't just copy the raw data because its 
        // layout is likely changing as a result of the 
        // schema change.
        Point p(*this);
        _header = header;

        std::vector<uint8_t> data;
        data.resize(wanted_length);
        data.assign(wanted_length, 0);
        _data = data;
        _header = header;
    
        setX(p.getX());
        setY(p.getY());
        setZ(p.getZ());
        

        // FIXME: copy other custom fields here?  resetting the 
        // headerptr can be catastrophic in a lot of cases.  
    } 


    // The header's scale/offset can change the raw storage of xyz.  
    // SetHeader can result in a rescaling of the data.
    double x;
    double y;
    double z;

    if (bApplyNewScaling)
    {
        x = getX();
        y = getY();
        z = getZ();        
    }

    _header = header;

    if (bApplyNewScaling)
    {
        setX(x);
        setY(y);
        setZ(z);
    }
    
}
Header const* Point::getHeader() const
{
    if (_header) 
        return _header;
     else 
        return &_default_header;
}

double Point::getX() const
{
    int32_t v = getRawX();
    
    double output  = (v * getHeader()->getScaleX()) + getHeader()->getOffsetX();
    
    return output;
}

double Point::getY() const
{
    int32_t v = getRawY();
    
    double output = (v * getHeader()->getScaleY()) + getHeader()->getOffsetY();
    return output;
}

double Point::getZ() const
{
    int32_t v = getRawZ();
    
    double output = 0;
    
    output  = (v * getHeader()->getScaleZ()) + getHeader()->getOffsetZ();
    
    return output;
}

void Point::setX( double const& value ) 
{
    int32_t v;
    double scale;
    double offset;

    scale = getHeader()->getScaleX();
    offset = getHeader()->getOffsetX();

    // descale the value given our scale/offset
    v = static_cast<int32_t>(detail::sround((value - offset) / scale));
    setRawX(v);
}

void Point::setY( double const& value ) 
{
    int32_t v;
    double scale;
    double offset;

    scale = getHeader()->getScaleY();
    offset = getHeader()->getOffsetY();


    // descale the value given our scale/offset
    v = static_cast<int32_t>(detail::sround((value - offset) / scale));
    setRawY(v);
}

void Point::setZ( double const& value ) 
{
    int32_t v;
    double scale;
    double offset;

    scale = getHeader()->getScaleZ();
    offset = getHeader()->getOffsetZ();

    // descale the value given our scale/offset
    v = static_cast<int32_t>(detail::sround((value - offset) / scale));
    setRawZ(v);
}

int32_t Point::getRawX() const
{
    std::vector<uint8_t>::size_type pos = 0;
    
#ifdef LIBHSL_ENDIAN_AWARE
    int32_t output = hsl::detail::bitsToInt<int32_t>(output, _data, pos);
    return output;
#else
    uint8_t* data = const_cast<uint8_t*>(&_data[0] + pos);
    int32_t* output = reinterpret_cast<int32_t*>(data);
    return *output;
#endif
}

int32_t Point::getRawY() const
{
    std::vector<uint8_t>::size_type pos = 4;

#ifdef LIBHSL_ENDIAN_AWARE
    int32_t output = hsl::detail::bitsToInt<int32_t>(output, _data, pos);
    return output;
#else
    uint8_t* data = const_cast<uint8_t*>(&_data[0] + pos);
    int32_t* output = reinterpret_cast<int32_t*>(data);
    return *output;
#endif
}

int32_t Point::getRawZ() const
{
    std::vector<uint8_t>::size_type pos = 8;

#ifdef LIBHSL_ENDIAN_AWARE
    int32_t output = hsl::detail::bitsToInt<int32_t>(output, _data, pos);
    return output;
#else
    uint8_t* data = const_cast<uint8_t*>(&_data[0] + pos);
    int32_t* output = reinterpret_cast<int32_t*>(data);
    return *output;
#endif
}

void Point::setRawX( int32_t const& value)
{  
    std::vector<uint8_t>::size_type pos = 0;     
    hsl::detail::intToBits<int32_t>(value, _data, pos);
}

void Point::setRawY( int32_t const& value)
{
    std::vector<uint8_t>::size_type pos = 4;
    hsl::detail::intToBits<int32_t>(value, _data, pos);
}

void Point::setRawZ( int32_t const& value)
{
    std::vector<uint8_t>::size_type pos = 8;    
    hsl::detail::intToBits<int32_t>(value, _data, pos);
}

size_t Point::getFieldBytePosition(std::size_t pos) const
{
    Field d;
    bool found = _header->getSchema().getField(pos, d);
    
    if (!found)
    {
        std::ostringstream oss;
        oss <<"Field at position " << pos << " not found";
        throw libhsl_error(oss.str());
    }
    return d.getByteOffset();
}

bool Point::getFieldBytePositionsById(FieldId id, FieldBytePositionArray &bytePositions) const
{
    FieldArray d;
    bool found = _header->getSchema().getFieldsById(id, d);
    
    if (!found)
    {
        std::ostringstream oss;
        oss <<"Field id " << id << " not found";
        return false;
    }

    for (size_t i = 0; i < d.size(); i++)
    {
        bytePositions.push_back(d[i].getByteOffset());
    }

    return true;
}

bool Point::getValuesById(FieldId id, VariantArray &values) const
{
	FieldArray fields;
	bool found = _header->getSchema().getFieldsById(id, fields);
    if (!found)
    {
        return false;
    }

	double scale = 1.0;
	double offset = 0.0;
	bool state = false;
	values.clear();
	for (size_t i = 0; i < fields.size(); i++)
    {
		Variant rawValue, value;
		if (getRawValueFromField(fields[i], rawValue))
		{
			if (fields[i].isScaled() || fields[i].isOffseted())
			{
				if (fields[i].isScaled())
					scale = fields[i].getScale();
				double offset = 0.0;
				if (fields[i].isOffseted())
					offset = fields[i].getOffset();

				if (!getScaledValue(rawValue, fields[i].getDataType(), value, scale, offset))
					return false;
			}
			else
			{
				value = rawValue;
			}
			values.push_back(value);
		}
		else
		{
			return false;
		}
    }

    return true;
}

bool Point::setValuesById(FieldId id, const VariantArray &values)
{
    FieldArray d;
    const Schema &schema = _header->getSchema();
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
        const Variant &value = values[i];
        Field &field = d[i];
        if (field.isScaled() || field.isOffseted())
        {
            if (field.isScaled())
                scale = field.getScale();
            double offset = 0.0;
            if (field.isOffseted())
                offset = field.getOffset();

            Variant rawValue;
            if(getScaledValue(value, field.getDataType(), rawValue, scale, offset))
                state = setRawValueToField(d[i], rawValue);
            else
                state = false;   
        }else
        {
            Variant rawValue = value;
            state = setRawValueToField(d[i], rawValue);
        }
    }

    return state;
}

bool Point::getValue(std::size_t index, Variant &value) const
{
    Field d;
    bool found = _header->getSchema().getField(index, d);
    if (!found)
    {
        return false;
    }

    getRawValueFromField(d, value);
    return true;
}

bool Point::setValue(size_t index, Variant &value)
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
            state = setRawValueToField(field, rawValue);
        else
            state = false;
    }
    else
        state = setRawValueToField(field, value);

    return state;
}

bool Point::getRawValueFromField(const Field &field, Variant &value) const
{
    size_t offset;
	switch (field.getDataType())
	{
	case DT_BIT:
		{
			offset = field.getByteOffset();
            size_t size = field.getByteSize();
            size_t bitOffset = field.getBitOffset();
            size_t sizeInBits = field.getBitSize();
            std::string bitsetValue;
            bool firstByte = true;
            size_t startBitPosition, stopBitPosition;
            boost::dynamic_bitset<> bitset(sizeInBits);
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
                }else if (i == field.getByteSize() - 1) // last byte
                {
                    startBitPosition = 0;
                    stopBitPosition = bitOffset;
                }else
                {
                    startBitPosition = 0;
                    stopBitPosition = 7;
                }
                
                uint8_t value = _data[offset + i];
                for (size_t j = startBitPosition; j <= stopBitPosition; j++)
                {
                    bitset[currentBit] = (value >> j) & 0x01;
                    currentBit++;
                }
            }

            value = bitset;    
		}
		break;
	case DT_CHAR:
		{
			offset = field.getByteOffset();
            //size_t size = field.getByteSize();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            int8_t* p_data = reinterpret_cast<int8_t*>(data);
            value = std::string((char*)p_data);
		}
		break;
	case DT_UCHAR:
		{
			offset = field.getByteOffset();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            value = *data;
		}		
		break;
	case DT_SHORT:
		{
			offset = field.getByteOffset();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            int16_t* p_data = reinterpret_cast<int16_t*>(data);
            value = *p_data;
		}		
		break;
	case DT_USHORT:
		{
			offset = field.getByteOffset();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            uint16_t* p_data = reinterpret_cast<uint16_t*>(data);
            value = *p_data;
		}	
		break;
	case DT_LONG:
		{
			offset = field.getByteOffset();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            int32_t* p_data = reinterpret_cast<int32_t*>(data);
            value = *p_data;
		}
		break;
	case DT_ULONG:
		{
			offset = field.getByteOffset();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            uint32_t* p_data = reinterpret_cast<uint32_t*>(data);
            value = *p_data;
		}		
		break;
	case DT_LONGLONG:
		{
			offset = field.getByteOffset();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            int64_t* p_data = reinterpret_cast<int64_t*>(data);
            value = *p_data;
		}			
		break;
	case DT_ULONGLONG:
		{
			offset = field.getByteOffset();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            uint64_t* p_data = reinterpret_cast<uint64_t*>(data);
            value = *p_data;
		}	
		break;
	case DT_FLOAT:
		{
			offset = field.getByteOffset();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            float* p_data = reinterpret_cast<float*>(data);
            value = *p_data;
		}					
		break;
	case DT_DOUBLE:
		{
			offset = field.getByteOffset();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            double* p_data = reinterpret_cast<double*>(data);
            value = *p_data;
		}						
		break;
	default:
		return false;
		break;
	}
	return true;
}

bool Point::setRawValueToField(const Field &field, const Variant &value)
{
    size_t offset, size;
    bool state = false;
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
                }else if (i == size - 1) // last byte
                {
                    startBitPosition = 0;
                    stopBitPosition = bitOffset;
                }else
                {
                    startBitPosition = 0;
                    stopBitPosition = 7;
                }
                
                uint8_t v = _data[offset + i];
    
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
				_data[offset + i] = v;
            }

            state = true;
		}
		break;
	case DT_CHAR:
		{
			offset = field.getByteOffset();
            size = field.getByteSize();
            uint8_t* data = const_cast<uint8_t *>(&_data[0] + offset);
            char* p_data = reinterpret_cast<char *>(data);
            size_t s;
            if(value.getByteSize(s))
            {
                if (s == size && value.getValue(p_data))
                    state = true;
            }
		}
		break;
	case DT_UCHAR:
		{
			offset = field.getByteOffset();
            size = field.getByteSize();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            uint8_t v;
            if (value.getValue(v))
            {
                *data = v;
                state = true;
            }           
		}		
		break;
	case DT_SHORT:
		{
			offset = field.getByteOffset();
            size = field.getByteSize();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            int16_t* p_data = reinterpret_cast<int16_t*>(data);
            int16_t v;
            if (value.getValue(v))
            {
                *p_data = v;
                state = true;
            }
		}		
		break;
	case DT_USHORT:
		{
			offset = field.getByteOffset();
            size = field.getByteSize();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            uint16_t* p_data = reinterpret_cast<uint16_t*>(data);
            uint16_t v;
            if (value.getValue(v))
            {
                *p_data = v;
                state = true;
            }
		}	
		break;
	case DT_LONG:
		{
			offset = field.getByteOffset();
            size = field.getByteSize();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            int32_t* p_data = reinterpret_cast<int32_t*>(data);
            int32_t v;
            if (value.getValue(v))
            {
                *p_data = v;
                state = true;
            }
		}
		break;
	case DT_ULONG:
		{
			offset = field.getByteOffset();
            size = field.getByteSize();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            uint32_t* p_data = reinterpret_cast<uint32_t*>(data);
            uint32_t v;
            if (value.getValue(v))
            {
                *p_data = v;
                state = true;
            }
		}		
		break;
	case DT_LONGLONG:
		{
			offset = field.getByteOffset();
            size = field.getByteSize();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            int64_t* p_data = reinterpret_cast<int64_t*>(data);
            int64_t v;
            if (value.getValue(v))
            {
                *p_data = v;
                state = true;
            }
		}			
		break;
	case DT_ULONGLONG:
		{
			offset = field.getByteOffset();
            size = field.getByteSize();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            uint64_t* p_data = reinterpret_cast<uint64_t*>(data);
            uint64_t v;
            if (value.getValue(v))
            {
                *p_data = v;
                state = true;
            }
		}	
		break;
	case DT_FLOAT:
		{
			offset = field.getByteOffset();
            size = field.getByteSize();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            float* p_data = reinterpret_cast<float*>(data);
            float v;
            if (value.getValue(v))
            {
                *p_data = v;
                state = true;
            }
		}					
		break;
	case DT_DOUBLE:
		{
			offset = field.getByteOffset();
            size = field.getByteSize();
            uint8_t* data = const_cast<uint8_t*>(&_data[0] + offset);
            double* p_data = reinterpret_cast<double*>(data);
            double v;
            if (value.getValue(v))
            {
                *p_data = v;
                state = true;
            }
		}						
		break;
	default:
		break;
	}

	return state;
}


bool Point::getWaveformDataByteOffset(uint64_t &offset) const
{
    VariantArray values;
    FieldId id = FI_ByteOffsetToWaveformData;
    bool hasId = getValuesById(id, values);
	if (!hasId)
		return false;

	bool state = values[0].getValue(offset);
    
    return state;
}

void Point::setWaveformDataAddress(uint64_t offset, uint32_t size)
{
    setWaveformDataByteOffset(offset);
    setWaveformDataSize(size);
}

void Point::setWaveformDataByteOffset(uint64_t offset)
{
    FieldBytePositionArray fa;
    size_t pos = 0;
    if (getFieldBytePositionsById(FI_ByteOffsetToWaveformData, fa))
    {
        pos = fa[0];  
        hsl::detail::intToBits<uint64_t>(offset, _data, pos);
    }
}

bool Point::getWaveformDataSize(uint32_t &size) const
{
    VariantArray values;
    FieldId id = FI_WaveformDataSize;
    bool hasId = getValuesById(id, values);
	if (!hasId)
		return false;

    bool state = values[0].getValue(size);    
    return state;
}

void Point::setWaveformDataSize(uint32_t size)
{
    FieldBytePositionArray fa;
    size_t pos = 0;
    if (getFieldBytePositionsById(FI_WaveformDataSize, fa))
    {
        pos = fa[0];  
        hsl::detail::intToBits<uint32_t>(size, _data, pos);
    }
}

uint16_t Point::getWaveformBandCount() const
{
    std::vector<uint8_t>::size_type pos = 0;

#ifdef LIBHSL_ENDIAN_AWARE
    uint16_t output = hsl::detail::bitsToInt<uint16_t>(output, _waveformData, pos);
    return output;
#else
    uint8_t* data = const_cast<uint8_t*>(&_waveformData[0] + pos);
    uint16_t* output = reinterpret_cast<uint16_t*>(data);
    return *output;
#endif
}

bool Point::getWaveformDescriptorIndexByBand(uint16_t band, uint16_t &index) const
{
    bool found = false;
    uint16_t count = getWaveformBandCount();

	std::vector<uint8_t>::size_type pos = sizeof(uint16_t);
	for (uint16_t i = 0; i < count; i++)
    {
        uint8_t* data = const_cast<uint8_t*>(&_waveformData[0] + pos);
        WaveformPacketDataDefinition* def = reinterpret_cast<WaveformPacketDataDefinition*>(data);
        if (def->bandIndex == band)
        {
            index = def->descriptorIndex;
            found = true;
            break;
        }
        pos += sizeof(WaveformPacketDataDefinition);
    }

    return found;
}

bool Point::getWaveformPacketDefinition(uint16_t band, WaveformPacketDataDefinition &d) const
{
    bool found = false;
    uint16_t count = getWaveformBandCount();

	std::vector<uint8_t>::size_type pos = sizeof(uint16_t);
    for (auto i = 0; i < count; i++)
    {
        uint8_t* data = const_cast<uint8_t*>(&_waveformData[0] + pos);
        WaveformPacketDataDefinition* def = reinterpret_cast<WaveformPacketDataDefinition*>(data);
        if (def->bandIndex == band)
        {
            d = *def;
            found = true;
            break;
        }
        pos += sizeof(WaveformPacketDataDefinition);
    }

    return found;
}

bool Point::getWaveformPacketDefinitions(std::vector<WaveformPacketDataDefinition> &wds) const
{
    bool found = false;
    uint16_t count = getWaveformBandCount();
    if (count > 0)
        found = true;

    std::vector<uint8_t>::size_type pos = sizeof(uint16_t);
	for (uint16_t i = 0; i < count; i++)
    {
        uint8_t* data = const_cast<uint8_t*>(&_waveformData[0] + pos);
        WaveformPacketDataDefinition* def = reinterpret_cast<WaveformPacketDataDefinition*>(data);
        wds.push_back(*def);
        pos += sizeof(WaveformPacketDataDefinition);
    }

    return found;
}

bool Point::getRawWaveformPacketData(uint16_t band, std::vector<uint8_t> &data) const
{
    WaveformPacketDataDefinition wd;
    
	if (!getWaveformPacketDefinition(band, wd))
		return false;

	uint64_t offset = 0;
	if (!getWaveformDataByteOffset(offset))
		return false; 

	offset = offset + wd.byteOffset;
    uint32_t size = wd.size;
    data.resize(size);
    memcpy(&data[0], &_waveformData[0] + offset, size);

    return true;
}

void Point::setWaveformData(const WaveformPacketRecord &record)
{
    record.toWaveformData(_waveformData);
}

bool Point::hasWaveformData() const
{
    return (_waveformData.size() > 0);
}

bool Point::getBandValues(size_t startBandIndex, size_t bandCount, unsigned char *data, size_t size) const
{
    size_t startIndex, stopIndex;
    bool state = false;

    // make sure we have sufficient data
    Schema & schema = const_cast<Header *>(_header)->getSchema();

    if(schema.getNthIndex(FI_BandValue, startBandIndex, startIndex) && 
        schema.getNthIndex(FI_BandValue, startBandIndex + bandCount - 1, stopIndex))
    {
        state = getData(startIndex, stopIndex, data, size);
    }

    return state;
}

bool copyData(const Field &field, const unsigned char *in, unsigned char *out, ScaleOffsetOp scaleInOut)
{
    bool state = false;

    switch (field.getDataType())
    {
    case DT_BIT:
        {
            const uint8_t *p_buf = reinterpret_cast<const uint8_t *>(in);
            uint8_t *p_data = reinterpret_cast<uint8_t *>(out);

            size_t byteSize = field.getByteSize();
            size_t bitOffset = field.getBitOffset();
            size_t sizeInBits = field.getBitSize();

            bool firstByte = true;
            size_t startBitPosition, stopBitPosition;
            size_t currentBit = 0;
            for (size_t i = 0; i < byteSize; i++)
            {
                if (firstByte)
                {
                    startBitPosition = bitOffset % 8;
                    stopBitPosition = 7;
                    firstByte = false;
                }else if (i == byteSize - 1) // last byte
                {
                    startBitPosition = 0;
                    stopBitPosition = (sizeInBits + bitOffset) % 8;
                }else
                {
                    startBitPosition = 0;
                    stopBitPosition = 7;
                }
                
                uint8_t v = *(p_data + i);
    
                uint8_t mask = 0;
                uint8_t bv = *(p_buf + i);
                for (size_t j = startBitPosition; j < stopBitPosition; j++)
                {
                    mask |= 0x1 << j;
                    currentBit++;
                }
                v &= ~mask;
                v |= mask & bv;
               *(p_data + i) = v;
            }

            state = true;
        }
        break;
    case DT_UCHAR:
        {
            const uint8_t *p_buf = reinterpret_cast<const uint8_t *>(in);

            // check if we need to do scale and offset data
            double scale = 1.0;
            double offset = 0.0;
            if (field.isScaled() || field.isOffseted())
            {
                // we need to scale data
                if (field.isScaled())
                    scale = field.getScale();
                if (field.isOffseted())
                    offset = field.getOffset();

                uint8_t *p_data = reinterpret_cast<uint8_t *>(out);
                getScaledValue<uint8_t>(*p_buf, *p_data, scale, offset, scaleInOut);
            }
            else
            {
                // just assign data
                uint8_t *p_data = reinterpret_cast<uint8_t *>(out);
                *p_data = *p_buf;
            }
            state = true;
        }
        break;
    case DT_SHORT:
        {
            const int16_t *p_buf = reinterpret_cast<const int16_t *>(in);

            // check if we need to do scale and offset data
            double scale = 1.0;
            double offset = 0.0;
            if (field.isScaled() || field.isOffseted())
            {
                // we need to scale data
                if (field.isScaled())
                    scale = field.getScale();
                if (field.isOffseted())
                    offset = field.getOffset();

                int16_t *p_data = reinterpret_cast<int16_t *>(out);
                getScaledValue<int16_t>(*p_buf, *p_data, scale, offset, scaleInOut);
            }
            else
            {
                // just assign data
                int16_t *p_data = reinterpret_cast<int16_t *>(out);
                *p_data = *p_buf;
            }
            state = true;
        }
        break;
    case DT_USHORT:
        {
            const uint16_t *p_buf = reinterpret_cast<const uint16_t *>(in);

            // check if we need to do scale and offset data
            double scale = 1.0;
            double offset = 0.0;
            if (field.isScaled() || field.isOffseted())
            {
                // we need to scale data
                if (field.isScaled())
                    scale = field.getScale();
                if (field.isOffseted())
                    offset = field.getOffset();

                uint16_t *p_data = reinterpret_cast<uint16_t *>(out);
                getScaledValue<uint16_t>(*p_buf, *p_data, scale, offset, scaleInOut);
            }
            else
            {
                // just assign data
                uint16_t *p_data = reinterpret_cast<uint16_t *>(out);
                *p_data = *p_buf;
            }
            state = true;
        }
        break;
    case DT_LONG:
        {
            const int32_t *p_buf = reinterpret_cast<const int32_t *>(in);

            // check if we need to do scale and offset data
            double scale = 1.0;
            double offset = 0.0;
            if (field.isScaled() || field.isOffseted())
            {
                // we need to scale data
                if (field.isScaled())
                    scale = field.getScale();
                if (field.isOffseted())
                    offset = field.getOffset();

                int32_t *p_data = reinterpret_cast<int32_t *>(out);
                getScaledValue<int32_t>(*p_buf, *p_data, scale, offset, scaleInOut);
            }
            else
            {
                // just assign data
                int32_t *p_data = reinterpret_cast<int32_t *>(out);
                *p_data = *p_buf;
            }
            state = true;
        }
        break;
    case DT_ULONG:
        {
            const uint32_t *p_buf = reinterpret_cast<const uint32_t *>(in);

            // check if we need to do scale and offset data
            double scale = 1.0;
            double offset = 0.0;
            if (field.isScaled() || field.isOffseted())
            {
                // we need to scale data
                if (field.isScaled())
                    scale = field.getScale();
                if (field.isOffseted())
                    offset = field.getOffset();

                uint32_t *p_data = reinterpret_cast<uint32_t *>(out);
                getScaledValue<uint32_t>(*p_buf, *p_data, scale, offset, scaleInOut);
            }
            else
            {
                // just assign data
                uint32_t *p_data = reinterpret_cast<uint32_t *>(out);
                *p_data = *p_buf;
            }
            state = true;
        }
        break;
    case DT_LONGLONG:
        {
            const int64_t *p_buf = reinterpret_cast<const int64_t *>(in);

            // check if we need to do scale and offset data
            double scale = 1.0;
            double offset = 0.0;
            if (field.isScaled() || field.isOffseted())
            {
                // we need to scale data
                if (field.isScaled())
                    scale = field.getScale();
                if (field.isOffseted())
                    offset = field.getOffset();

                int64_t *p_data = reinterpret_cast<int64_t *>(out);
                getScaledValue<int64_t>(*p_buf, *p_data, scale, offset, scaleInOut);
            }
            else
            {
                // just assign data
                int64_t *p_data = reinterpret_cast<int64_t *>(out);
                *p_data = *p_buf;
            }
            state = true;
        }
        break;
    case DT_ULONGLONG:
        {
            const uint64_t *p_buf = reinterpret_cast<const uint64_t *>(in);

            // check if we need to do scale and offset data
            double scale = 1.0;
            double offset = 0.0;
            if (field.isScaled() || field.isOffseted())
            {
                // we need to scale data
                if (field.isScaled())
                    scale = field.getScale();
                if (field.isOffseted())
                    offset = field.getOffset();

                uint64_t *p_data = reinterpret_cast<uint64_t *>(out);
                getScaledValue<uint64_t>(*p_buf, *p_data, scale, offset, scaleInOut);
            }
            else
            {
                // just assign data
                uint64_t *p_data = reinterpret_cast<uint64_t *>(out);
                *p_data = *p_buf;
            }
            state = true;
        }
        break;
    case DT_FLOAT:
        {
            const float *p_buf = reinterpret_cast<const float *>(in);

            // check if we need to do scale and offset data
            double scale = 1.0;
            double offset = 0.0;
            if (field.isScaled() || field.isOffseted())
            {
                // we need to scale data
                if (field.isScaled())
                    scale = field.getScale();
                if (field.isOffseted())
                    offset = field.getOffset();

                float *p_data = reinterpret_cast<float *>(out);
                getScaledValue<float>(*p_buf, *p_data, scale, offset, scaleInOut);
            }
            else
            {
                // just assign data
                float *p_data = reinterpret_cast<float *>(out);
                *p_data = *p_buf;
            }
            state = true;
        }
        break;
    case DT_DOUBLE:
        {
            const double *p_buf = reinterpret_cast<const double *>(in);

            // check if we need to do scale and offset data
            double scale = 1.0;
            double offset = 0.0;
            if (field.isScaled() || field.isOffseted())
            {
                // we need to scale data
                if (field.isScaled())
                    scale = field.getScale();
                if (field.isOffseted())
                    offset = field.getOffset();

                double *p_data = reinterpret_cast<double *>(out);
                getScaledValue<double>(*p_buf, *p_data, scale, offset, scaleInOut);
            }
            else
            {
                // just assign data
                double *p_data = reinterpret_cast<double *>(out);
                *p_data = *p_buf;
            }
            state = true;
        }
        break;
    default:
        state = false;
        break;
    }

    return state;
}

bool Point::getData(size_t startIndex, size_t stopIndex, unsigned char *data, size_t size) const
{
    if (startIndex >= _header->getSchema().getFieldCount() || startIndex >= _header->getSchema().getFieldCount())
        return false;

    if (startIndex > stopIndex)
    {
        size_t k = startIndex;
        startIndex = stopIndex;
        stopIndex = k;
    }

    size_t outPos = 0;
    ScaleOffsetOp scaleInOut = SO_Out;
    for (size_t i = startIndex; i <= stopIndex; i++)
    {
        Field field;
        _header->getSchema().getField(i, field); 
        size_t inPos = field.getByteOffset();
        if (!copyData(field, _data.data() + inPos, data + outPos / 8, scaleInOut))
            return false;    
        outPos += field.getBitSize();
    }

    return true;    
}

bool Point::setData(size_t startIndex, size_t stopIndex, const unsigned char *data, size_t size)
{
    if (startIndex >= _header->getSchema().getFieldCount() || startIndex >= _header->getSchema().getFieldCount())
        return false;

    if (startIndex > stopIndex)
    {
        size_t k = startIndex;
        startIndex = stopIndex;
        stopIndex = k;
    }
    
    ScaleOffsetOp scaleInOut = SO_In;
    size_t inPos = 0;
    for (size_t i = startIndex; i <= stopIndex; i++)
    {
        Field field;
        _header->getSchema().getField(i, field); 
        size_t outPos = field.getByteOffset();
        if (!copyData(field, data + inPos / 8, _data.data() + outPos, scaleInOut))
            return false;    
        inPos += field.getBitSize();
    }

    return true;
}

bool Point::setBandValues(size_t startBandIndex, size_t bandCount, const unsigned char *data, size_t size)
{
    size_t startIndex, stopIndex;
    bool state = false;

    // make sure we have sufficient data
    Schema & schema = const_cast<Header *>(_header)->getSchema();
    schema.calculateSizes();

    if(schema.getNthIndex(FI_BandValue, startBandIndex, startIndex) && 
        schema.getNthIndex(FI_BandValue, startBandIndex + bandCount - 1, stopIndex))
    {
        state = setData(startIndex, stopIndex, data, size);
    }

    return state;
}


}
