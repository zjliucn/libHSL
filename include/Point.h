/*************************************************************************************
 * 
 * 
 * Copyright (c) 2021, Zhengjun Liu <zjliu@casm.ac.cn>
 * Copyright (c) 2008, Mateusz Loskot <mateusz@loskot.net>
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

#include <stdexcept>
#include <cstdlib>
#include <vector>
#include "hslLIB.h"
#include "hslDefinitions.h"
#include "Variant.h"
#include "WaveformPacketRecord.h"

namespace hsl 
{

class Header;
class Field;

enum ScaleOffsetOp
{
    SO_In,
    SO_Out
};

/// Point data record contains X, Y, Z coordinates and variable attributes.
class LIBHSL_API Point
{
public:
//    Point();
    Point(const Header * header);
    Point(const Point & other);
    ~Point() {};
    Point& operator=(const Point & rhs);

    /// Returns the scaled and shifted X-coordinate. Scaling and shifting (offset) parameters are defined in the header.
    double getX() const;
    /// Returns the scaled and shifted Y-coordinate. Scaling and shifting (offset) parameters are defined in the header.
    double getY() const;
    /// Returns the scaled and shifted Z-coordinate. Scaling and shifting (offset) parameters are defined in the header.
    double getZ() const;
    
    int32_t getRawX() const;
    int32_t getRawY() const;
    int32_t getRawZ() const;
        
    void setCoordinates(double const& x, double const& y, double const& z);
    
    void setX(double const& value);
    void setY(double const& value);
    void setZ(double const& value);

    void setRawX(int32_t const& value);
    void setRawY(int32_t const& value);
    void setRawZ(int32_t const& value);

    /// Const version of index operator providing access to XYZ coordinates of point record.
    /// Valid index values are 0, 1 or 2.
    /// \exception std::out_of_range if requested index is out of range (> 2).
    double operator[](std::size_t const& index) const;

    /// \todo TODO: Should we compare other data members, but not only coordinates?
    bool equal(Point const& other) const;

    bool validate() const;
    bool isValid() const;

    std::vector<uint8_t> const& getData() const {return _data; }
    std::vector<uint8_t> & getData() {return _data; }
    void setData(std::vector<uint8_t> const& v) { _data = v;}

    // get data according to the specified position range of the point, note: data buffer should
    // reset to 0 before calling getData() if point has bitset field, and the output data are 
    // aligned according to the intrinsic data type of the point schema
    bool getData(size_t startIndex, size_t stopIndex, unsigned char *data, size_t size) const;

    // set data according to the specified position range of the point, note: data should
    // be aligned according to the intrinsic data type of the point schema
    bool setData(size_t startIndex, size_t stopIndex, const unsigned char *data, size_t size);
    
    void setWaveformDataAddress(uint64_t offset, uint32_t size);
    void setWaveformDataByteOffset(uint64_t offset);
	bool getWaveformDataByteOffset(uint64_t &offset) const;

    void setWaveformDataSize(uint32_t size);
	bool getWaveformDataSize(uint32_t &size) const;

    bool hasWaveformData() const;
    std::vector<uint8_t> const& getWaveformData() const {return _waveformData; }
    std::vector<uint8_t> & getWaveformData() {return _waveformData; }
    void setWaveformData(std::vector<uint8_t> const& v) { _waveformData = v;}
    void setWaveformData(const WaveformPacketRecord &record);

    uint16_t getWaveformBandCount() const;
    bool getWaveformDescriptorIndexByBand(uint16_t band, uint16_t &index) const;
    bool getWaveformPacketDefinition(uint16_t band, WaveformPacketDataDefinition &d) const;
    bool getWaveformPacketDefinitions(std::vector<WaveformPacketDataDefinition> &wds) const;

	bool getRawWaveformPacketData(uint16_t band, std::vector<uint8_t> &data) const;

    void setHeader(Header const* header); 
    Header const* getHeader() const;

    bool getValuesById(FieldId id, VariantArray &values) const;
    bool setValuesById(FieldId id, const VariantArray &values);

    bool getValue(size_t index, Variant &value) const;
    bool setValue(size_t index, Variant &value);

    bool getBandValues(size_t startBandIndex, size_t bandCount, unsigned char *data, size_t size) const;
    bool setBandValues(size_t startBandIndex, size_t bandCount, const unsigned char *data, size_t size);
    
private:
    typedef std::vector<size_t> FieldBytePositionArray;

    size_t getFieldBytePosition(std::size_t pos) const;
    bool getFieldBytePositionsById(FieldId id, FieldBytePositionArray &bytePositions) const;

    bool getRawValueFromField(const Field &field, Variant &value) const;
    bool setRawValueToField(const Field &field, const Variant &value);

private:
    std::vector<uint8_t>    _data;
    std::vector<uint8_t>    _waveformData;    
    Header const*           _header;
    Header const&           _default_header;
};

/// Equal-to operator implemented in terms of Point::equal method.
inline bool operator==(Point const& lhs, Point const& rhs)
{
    return lhs.equal(rhs);
}

/// Not-equal-to operator implemented in terms of Point::equal method.
inline bool operator!=(Point const& lhs, Point const& rhs)
{
    return (!(lhs == rhs));
}

inline double Point::operator[](std::size_t const& index) const
{
    
    if (index == 0) 
        return getX();
    if (index == 1) 
        return getY();
    if (index == 2)
        return getZ();

    throw std::out_of_range("coordinate subscript out of range");
    
}

typedef std::shared_ptr<Point> PointPtr;

}