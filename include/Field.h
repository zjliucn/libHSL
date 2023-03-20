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
// std
#include <iosfwd>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>
// boost
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include "hslLIB.h"
#include "hslDefinitions.h"
#include "IdDefinitions.h"
#include "Variant.h"

namespace hsl {

enum ScaleOffsetOp
{
    SO_In,
    SO_Out
};

/// Field definition
class LIBHSL_API Field
{
public:
    Field();
    Field(FieldId id, std::string const& name, DataType type, std::size_t sizeInBits);
    Field& operator=(Field const& rhs);
    Field(Field const& other);

    bool operator==(const Field& other) const;
    bool operator!=(const Field& other) const { return !(*this == other); }
        
    virtual ~Field() {}

    inline DataType getDataType() const { return _dataType; }
    inline void setDataType(DataType type);    

    inline FieldId getId() const { return _id; }
    inline void setId(FieldId id);

    inline void setName(std::string const& name);
    inline std::string const& getName() const { return _name; }

    inline void setBitSize(std::size_t size_in_bits);        

    /// bits, total logical size of point record, including any custom
    /// fields
    inline size_t getBitSize() const 
    {
        return _bitSize;
    }
    
    /// bytes, physical/serialisation size of record
    size_t getByteSize() const;

    /// The byte location to start reading/writing 
    /// point data from in a composited schema.  liblas::Schema 
    /// will set these values for you when liblas::Field are 
    /// added to the liblas::Schema.
    inline size_t getByteOffset() const 
    {
        return _byteOffset;
    }

    inline void setByteOffset(size_t v) 
    {
        _byteOffset = v;
    }

    /// The bit location within the byte to start reading data.  Schema 
    /// will set these values for you when Field are added to the Schema. 
    // This value will be 0 for fields that are composed of entire bytes.
    inline size_t getBitOffset() const 
    {
        return _bitOffset;
    }

    inline void setBitOffset(size_t v) 
    {
        _bitOffset = v;
    }

    inline bool isValid() const;

    /// Is this field required by PointFormat
    inline bool isRequired() const { return _isRequired; }
    inline void isRequired(bool v) { _isRequired = v; }

    /// Is this field being used.  A field with 
    /// IsActive false may exist as a placeholder in PointFormat-specified
    /// fields, but have their IsActive flag set to false.  In this 
    /// case, those values may be disregarded.
    inline bool isActive() const { return _isActive; }
    inline void isActive(bool v) { _isActive = v; }

    inline std::string getDescription() const { return _description; }
    inline void setDescription(std::string const& v) { _description = v; }

    /// Is this field is numeric.  Fields with IsNumeric == false
    /// are considered generic bit/byte fields/
    inline bool isNumeric() const { return _isNumeric ; }
    inline void isNumeric(bool v) { _isNumeric = v; }

    /// Does this field have a sign?  Only applicable to fields with 
    /// IsNumeric == true.
    inline bool isSigned() const { return _isSigned; }
    inline void isSigned(bool v) { _isSigned = v; }

    /// Does this field interpret to an integer?  Only applicable to fields 
    /// with IsNumeric == true.
    inline bool isInteger() const { return _isInteger; }
    inline void isInteger(bool v) { _isInteger = v; }

    /// The minimum value of this field as a double
    inline double getMinimum() const { return _min; }
    inline void setMinimum(double min) { _min = min; }
    
    /// The maximum value of this field as a double
    inline double getMaximum() const { return _max; }
    inline void setMaximum(double max) { _max = max; }
    
    /// The index position of the index.  In a standard PF_PointFormat0 
    /// data record, the X field would have a position of 0, while 
    /// the Y field would have a position of 1, for example.
    inline uint32_t getPosition() const { return _position; }
    inline void setPosition(uint32_t v) { _position = v; }

    bool isScaled() const;
    void isScaled(bool v);

    bool isOffseted() const;
    void isOffseted(bool v);
    
    /// The scaling value for this field as a double.  This should 
    /// be positive or negative powers of ten.
    inline double getScale() const { return _scale; }
    inline void setScale(double v) { _scale = v; }
    
    /// The offset value for this field.  Usually zero, but it 
    /// can be set to any value in combination with the scale to 
    /// allow for more expressive ranges.
    inline double getOffset() const { return _offset; }
    inline void setOffset(double v) { _offset = v; }
    
    /// If true, this field uses scale/offset values 
    inline bool isFinitePrecision() const { return _precise; }
    inline void isFinitePrecision(bool v) { _precise = v; }
    
    inline bool operator < (Field const& field) const 
    {
        return _position < field._position;
    }
    inline bool operator > (Field const& field) const 
    {
        return _position > field._position;
    }

private:
    DataType        _dataType;
    FieldId         _id;       
    std::string     _name;
    std::size_t     _bitSize;
    bool            _isRequired;
    bool            _isActive;
    std::string     _description;
    double          _min;
    double          _max;
    bool            _isNumeric;
    bool            _isSigned;
    bool            _isInteger;
    bool            _isScaled;
    bool            _isOffseted;
    uint32_t        _position;
    double          _scale;
    double          _offset;
    bool            _precise;
    std::size_t     _byteOffset;
    std::size_t     _bitOffset;   
  
};

typedef Field Band;


struct SetRequired
{
    SetRequired(bool req) : req_(req) {}

    void operator()(Field& e)
    {
        e.isRequired(req_);
    }

private:
    bool req_;
};

struct SetActive
{
    SetActive(bool req) : req_(req) {}

    void operator()(Field& e)
    {
        e.isActive(req_);
    }

private:
    bool req_;
};

// descale the value given our scale/offset
template <typename T>
void getScaledValue(const T& valueIn, T& valueOut, double scale, double offset, ScaleOffsetOp scaleInOut)
{
    if (scaleInOut == SO_In)
    {
        valueOut = static_cast<T>(detail::sround((valueIn - offset) / scale));
    }
    else if (scaleInOut == SO_Out)
    {
        valueOut = static_cast<T>(detail::sround(scale * valueIn + offset));
    }
};

bool getScaledValue(const Variant& value, DataType rawType, Variant& rawValue, double scale, double offset);

}
