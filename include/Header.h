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

#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cmath>
#include <memory>
#include "Singleton.h"
#include "hslDefinitions.h"
#include "Schema.h"
#include "Bounds.h"

namespace hsl
{

/// Definition of the header block that contains set of generic data and metadata
/// describing the hyperspectral point cloud files. The header is stored
/// at the beginning of every valid *.hsp file and its georeference file named *.prj.
///
///
class LIBHSL_API Header
{
public:
    /// Default constructor.
    /// The default constructed header is configured similar to the ASPRS
    /// LAS 1.2 Specification, point data format set to 0.
    /// Other fields filled with 0.
    Header();
    Header(PointFormat format);
    Header(const Schema &schema);
    
    /// Copy constructor.
    Header(Header const& other);

    /// Assignment operator.
    Header& operator=(Header const& rhs);
    
    /// Comparison operator.
    bool operator==(const Header& other) const;

    bool isCompatible(const Header& other) const;

    /// Get file signature.
    /// \return 5-characters long string - \b "HSPCD".
    std::string getFileSignature() const;

    /// Set file signature.
    /// The only value allowed as file signature is \b "HSPCD",
    /// defined as FileSignature constant.
    /// \exception std::invalid_argument - if invalid signature given.
    /// \param v - string contains file signature, at least 5-bytes long
    /// with "HSPCD" as first four bytes.
    void setFileSignature(std::string const& v);

    /// Get major component of version of HSP format.
    /// \return Always 1 is returned as the only valid value.
    uint8_t getVersionMajor() const;

    /// Set major component of version of HSP format.
    /// \exception std::out_of_range - invalid value given.
    /// \param v - value between eVersionMajorMin and eVersionMajorMax.
    void setVersionMajor(uint8_t v);

    /// Get minor component of version of HSP format.
    /// \return Valid values are 0, 1, 2, 3.
    uint8_t getVersionMinor() const;

    /// Set minor component of version of HSP format.
    /// \exception std::out_of_range - invalid value given.
    /// \param v - value between FV_VersionMinorMin and FV_VersionMinorMax.
    void setVersionMinor(uint8_t v);
   
    /// Get number of bytes from the beginning to the first point record.
    uint32_t getDataOffset() const;

    /// Set number of bytes from the beginning to the first point record.
    void setDataOffset(uint32_t v);

    /// Initialize point data format from predefined format.
    void setDataFormat(PointFormat v);

    /// The length in bytes of each point.  All points in the file are 
    /// considered to be fixed in size, and the PointFormat is used 
    /// to determine the fixed portion of the dimensions in the point.  Any 
    /// other byte space in the point record beyond the liblas::Schema::GetBaseByteSize() 
    /// can be used for other, optional, dimensions.
    uint32_t getDataRecordLength() const;
    
    /// Get total number of point records stored in the *.hsp file.
    uint64_t getPointRecordsCount() const;

    /// Set number of point records that will be stored in a new *.hsp file.
    void setPointRecordsCount(uint64_t v);

    /// Set number of returns that will be stored in a new *.hsp file.
    void setReturnCount(uint32_t v);

    /// Get the return count of the total point records
    uint32_t getReturnCount() const;

    /// Set values of total point records per return.
    /// \param index - subscript of array element being updated.
    /// \param v - new value to assign to array element identified by index.
    bool setPointRecordByReturn(std::size_t index, uint64_t v);

    uint64_t getPointRecordByReturn(std::size_t index) const;

    std::vector<uint64_t> getPointRecordsByReturnCount() const;
    
    /// Get scale factor for X coordinate.
    double getScaleX() const;

    /// Get scale factor for Y coordinate.
    double getScaleY() const;
    
    /// Get scale factor for Z coordinate.
    double getScaleZ() const;

    /// Set values of scale factor for X, Y and Z coordinates.
    void setScale(double x, double y, double z);

    /// Get X coordinate offset.
    double getOffsetX() const;
    
    /// Get Y coordinate offset.
    double getOffsetY() const;
    
    /// Get Z coordinate offset.
    double getOffsetZ() const;

    /// Set values of X, Y and Z coordinates offset.
    void setOffset(double x, double y, double z);

    /// Get minimum value of extent of X coordinate.
    double getMaxX() const;

    /// Get maximum value of extent of X coordinate.
    double getMinX() const;

    /// Get minimum value of extent of Y coordinate.
    double getMaxY() const;

    /// Get maximum value of extent of Y coordinate.
    double getMinY() const;

    /// Get minimum value of extent of Z coordinate.
    double getMaxZ() const;

    /// Get maximum value of extent of Z coordinate.
    double getMinZ() const;

    /// Set maximum values of extent of X, Y and Z coordinates.
    void setMax(double x, double y, double z);

    /// Set minimum values of extent of X, Y and Z coordinates.
    void setMin(double x, double y, double z);
    
    /// Returns the schema.
    const Schema & getSchema() const;
    Schema & getSchema();

    /// Sets the schema
    void setSchema(const Schema& schema);

    /// Return the Bounds.  This is a 
    /// combination of the GetMax and GetMin 
    /// (or getMinX, getMaxY, etc) data.
    const Bounds<double>& getExtent() const;

    /// Set the Bounds.  This is a combination of the GetMax and GetMin 
    /// (or getMinX, getMaxY, etc) data, and it is equivalent to setting 
    /// all of these values.
    void setExtent(Bounds<double> const& extent);
    
    /// Returns true if the file is compressed,
    /// as determined by the high bit in the point type
    bool isCompressed() const;

    /// Sets whether or not the points are compressed.
    void setCompressed(bool b);

    bool hasWaveformData() const;

	bool addWaveformPacketDesc(const WaveformPacketDesc &descriptor);

    bool isInternalWaveformData() const;   
    void setInternalWaveformData(bool b);

    bool isInternalBandData() const;  
    void setInternalBandData(bool b);

	std::shared_ptr<FileHeader>	getFileHeader() const { return _fileHeader; }
	void setFileHeader(std::shared_ptr<FileHeader> fileHeader) { _fileHeader = fileHeader; }

	std::shared_ptr<BlockDesc> getBlockDesc() const { return _blockDesc; }
	void setBlockDesc(std::shared_ptr<BlockDesc> blockDesc) { _blockDesc = blockDesc; }

	std::shared_ptr<WaveformDesc> getWaveformDesc() const { return _waveformDesc; }
	void setWaveformDesc(std::shared_ptr<WaveformDesc> waveformDesc) { _waveformDesc = waveformDesc; }

	void updateHeader();
    
private:
    void init();
    size_t calculateHeaderSize();

private:
    static const unsigned short     _FileSignatureSize;
    static const std::string        _FileSignature;
	std::shared_ptr<FileHeader>		_fileHeader;
    std::vector<uint64_t>           _pointRecordsByReturn;
	std::shared_ptr<BlockDesc>		_blockDesc;
	std::shared_ptr<WaveformDesc>	_waveformDesc;
    Schema                          _schema;
 
    bool                            _isCompressed;
};

/// Singleton used for all empty points upon construction.  If 
/// a reader creates the point, the HeaderPtr from the file that was 
/// read will be used, but all stand-alone points will have EmptyHeader 
/// as their base.
class LIBHSL_API DefaultHeader : public Singleton<Header>
{
public:
    ~DefaultHeader() {}


protected:
    DefaultHeader();
    DefaultHeader( DefaultHeader const&);
    DefaultHeader& operator=( DefaultHeader const&);
    
};

typedef std::shared_ptr<Header> HeaderPtr;
typedef std::shared_ptr<const Header> ConstHeaderPtr;

}