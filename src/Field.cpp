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


#include "Field.h"
#include <algorithm>
#include <sstream>
#include "Schema.h"
#include "detail/private_utility.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4512)
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif


namespace hsl
{ 
Field::Field() : _dataType(DT_UNKNOWN), _id(FI_UNKNOWN), _name(""), _bitSize(0), _isRequired(false),
    _isActive(false), _description(std::string("")), _min(0), _max(0), _isNumeric(false), _isSigned(false), _isInteger(false),
    _isScaled(false), _isOffseted(false), _position(0), _scale(1.0), _offset(0.0), _precise(true), _byteOffset(0), _bitOffset(0)
{
}

Field::Field(FieldId id, std::string const& name, DataType type, std::size_t sizeInBits) : _dataType(type), 
    _id(id), _name(name), _bitSize(sizeInBits), _isRequired(false), _isActive(false), _description(std::string("")), 
    _min(0), _max(0), _isNumeric(false), _isSigned(false), _isInteger(false), _isScaled(false), _isOffseted(false), _position(0), 
    _scale(1.0), _offset(0.0), _precise(true), _byteOffset(0), _bitOffset(0)
{
     if (sizeInBits == 0)
	 {
        throw std::runtime_error("The bit size of the field is 0, the field is invalid.");
     }
}

Field::Field(Field const& other) : _dataType(other._dataType), _id(other._id), _name(other._name), 
    _bitSize(other._bitSize), _isRequired(other._isRequired), _isActive(other._isActive), _description(other._description), 
    _min(other._min), _max(other._max), _isNumeric(other._isNumeric), _isSigned(other._isSigned), _isInteger(other._isInteger), 
    _isScaled(other._isScaled), _isOffseted(other._isOffseted), _position(other._position), _scale(other._scale), _offset(other._offset), _precise(other._precise), 
    _byteOffset(other._byteOffset), _bitOffset(other._bitOffset)
{
}
 
Field& Field::operator=(Field const& rhs)
{
    if (&rhs != this)
    {
        _dataType = rhs._dataType;
        _id = rhs._id;
        _name = rhs._name;
        _bitSize = rhs._bitSize;
        _isRequired = rhs._isRequired;
        _isActive = rhs._isActive;
        _description = rhs._description;
        _min = rhs._min;
        _max = rhs._max;
        _isNumeric = rhs._isNumeric;
        _isSigned = rhs._isSigned;
        _isInteger = rhs._isInteger;
        _isScaled = rhs._isScaled;
        _isOffseted = rhs._isOffseted;        
        _position = rhs._position;
        _scale =  rhs._scale;
        _offset = rhs._offset;
        _precise = rhs._precise;
        _byteOffset = rhs._byteOffset;
        _bitOffset = rhs._bitOffset;
    }
    
    return *this;
}

size_t Field::getByteSize() const 
{
    size_t const pos = _bitSize % 8;
    if (pos > 0)
	{
        // For fields that are not byte aligned,
        // we need to determine how many bytes they 
        // will take.  We have to read at least one byte if the 
        // size in bits is less than 8.  If it is more than 8, 
        // we need to read the number of bytes it takes + 1 extra.
        if (_bitSize > 8)
        {
            return _bitSize / 8 + 1;
        } else 
        {
            return 1;
        }
    }
    return _bitSize / 8;
}

bool Field::operator==(const Field& other) const
{

    if (&other == this) return true;
    if (_dataType != other._dataType) return false;
    if (_id != other._id) return false;
    if (_name != other._name) return false;
    if (_bitSize != other._bitSize) return false;
    if (_isRequired != other._isRequired) return false;
    if (_isActive != other._isActive) return false;
    if (_description != other._description) return false;
    if (!detail::compare_distance(_min, other._min)) return false;
    if (!detail::compare_distance(_max, other._max)) return false;
    if (_isNumeric != other._isNumeric) return false;
    if (_isSigned != other._isSigned) return false;
    if (_isInteger != other._isInteger) return false;
    if (_isScaled != other._isScaled) return false;   
    if (_isOffseted != other._isOffseted) return false;       
    if (_position != other._position) return false;
    if (_scale !=  other._scale) return false;
    if (_offset != other._offset) return false;
    if (_precise != other._precise) return false;    
    if (_byteOffset != other._byteOffset) return false;
    if (_bitOffset != other._bitOffset) return false;
    
    return true;

}

 bool Field::isScaled() const
 {
     if (_dataType == DT_BIT || _dataType == DT_CHAR || _dataType == DT_UNKNOWN)
        return false;
    else
    {
        return _isScaled;
    }    
 }

void Field::isScaled(bool v)
{
     if (_dataType == DT_BIT || _dataType == DT_CHAR || _dataType == DT_UNKNOWN)
        _isScaled = false;
    else
    {
        _isScaled = v;
    }   
}

bool Field::isOffseted() const
 {
     if (_dataType == DT_BIT || _dataType == DT_CHAR || _dataType == DT_UNKNOWN)
        return false;
    else
    {
        return _isOffseted;
    }    
 }

void Field::isOffseted(bool v)
{
     if (_dataType == DT_BIT || _dataType == DT_CHAR || _dataType == DT_UNKNOWN)
        _isOffseted = false;
    else
    {
        _isOffseted = v;
    }   
}

void Field::setId(FieldId id)
{
    _id = id;
}

void Field::setName(std::string const& name)
{
    if (name == "")
	{
       throw std::runtime_error("The name of the field is null, the field is invalid.");
    }
    _name = name;
}

void Field::setBitSize(std::size_t sizeInBits)
{
    if (sizeInBits == 0)
	{
       throw std::runtime_error("The bit size of the field is 0, the field is invalid.");
    }
    _bitSize = sizeInBits;
   
}

void Field::setDataType(DataType type)
{
    _dataType = type;
}

bool Field::isValid() const
{
    if (_name != "" && _bitSize != 0 && _dataType != DT_UNKNOWN && _dataType < DT_RESERVED)
        return true;
    else
        return false;    
}

}
