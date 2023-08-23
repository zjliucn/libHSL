/*************************************************************************************
 * 
 * 
 * Copyright (c) 2021, Zhengjun Liu <zjliu@casm.ac.cn>
 * Copyright (c) 2010, Howard Butler <hobu.inc@gmail.com>
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


#include "Schema.h"
#include "detail/private_utility.hpp"
#include "Exception.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4512)
#endif


#ifdef _MSC_VER
#pragma warning(pop)
#endif
// boost
#include <boost/cstdint.hpp>
// std
#include <algorithm>
#include <sstream>

using namespace boost;

namespace hsl { 


Schema::Schema(PointFormat pointFormat): _pointFormat(pointFormat), _nextPosition(0), _bitSize(0), _baseBitSize(0), _schemaVersion(1)
{
    updateRequiredFields(pointFormat);
}

/// copy constructor
Schema::Schema(Schema const& other) : _pointFormat(other._pointFormat), _nextPosition(other._nextPosition), 
    _bitSize(other._bitSize), _baseBitSize(other._baseBitSize), _schemaVersion(other._schemaVersion), 
    _index(other._index)
{
}

// assignment constructor
Schema& Schema::operator=(Schema const& rhs)
{
    if (&rhs != this)
    {
        _pointFormat = rhs._pointFormat;
        _nextPosition = rhs._nextPosition;
        _index = rhs._index;
        _baseBitSize = rhs._baseBitSize;
        _bitSize = rhs._bitSize;
        _schemaVersion = rhs._schemaVersion;
    }
    
    return *this;
}

void Schema::addXYZ()
{
    std::ostringstream text;
    
    Field x(FI_X, "X", DT_ULONG, 32);
    text << "x coordinate as a long integer.  You must use the scale and "
         << "offset information of the header to determine the double value.";
    x.setDescription(text.str()); 
    x.isInteger(true);
    x.isNumeric(true);
    x.isSigned(true);
    x.isRequired(true);
    x.isActive(true);

    addField(x);
    text.str("");

    Field y(FI_Y, "Y", DT_ULONG, 32);
    text << "y coordinate as a long integer.  You must use the scale and "
         << "offset information of the header to determine the double value.";
    y.setDescription(text.str()); 
    y.isInteger(true);
    y.isNumeric(true);
    y.isSigned(true);
    y.isRequired(true);
    y.isActive(true);

    addField(y);
    text.str("");
    
    Field z(FI_Z, "Z", DT_ULONG, 32);
    text << "z coordinate as a long integer.  You must use the scale and "
         << "offset information of the header to determine the double value.";
    z.setDescription(text.str()); 
    z.isInteger(true);
    z.isNumeric(true);
    z.isSigned(true);
    z.isRequired(true);
    z.isActive(true);

    addField(z);
    text.str("");
}

void Schema::addWaveform()
{
    std::ostringstream text;
    
    Field byteOffset(FI_ByteOffsetToWaveformData, "WaveformOffset", DT_ULONGLONG, 64);
    text << "byte offset of waveform data as a unsigned long long integer without scale and offset.";
    byteOffset.setDescription(text.str()); 
    byteOffset.isInteger(true);
    byteOffset.isNumeric(true);
    byteOffset.isSigned(false);
    byteOffset.isRequired(true);
    byteOffset.isActive(true);
    byteOffset.isScaled(false);    
    byteOffset.isOffseted(false);

    addField(byteOffset);
    text.str("");

    Field waveformSize(FI_WaveformDataSize, "WaveformSize", DT_ULONG, 32);
    text << "wave form data size as a unsigned long integer.  You must use without scale and offset.";
    waveformSize.setDescription(text.str()); 
    waveformSize.isInteger(true);
    waveformSize.isNumeric(true);
	waveformSize.isSigned(false);
    waveformSize.isRequired(true);
    waveformSize.isActive(true);
    waveformSize.isScaled(false);    
    waveformSize.isOffseted(false);

    addField(waveformSize);
}

void Schema::addASPRSBasicLasFields()
{
    std::ostringstream text;

    Field intensity(FI_Intensity, "Intensity", DT_USHORT, 16);
    text << "The intensity value is the integer representation of the pulse " 
            "return magnitude. This value is optional and system specific. "
            "However, it should always be included if available.";
    intensity.setDescription(text.str());
    intensity.isInteger(true);
    intensity.isNumeric(true);
    addField(intensity);
    text.str("");

    Field return_no(FI_ReturnNumber, "Return Number", DT_BIT, 4);
    text << "Return Number: The Return Number is the pulse return number for "
            "a given output pulse. A given output laser pulse can have many "
            "returns, and they must be marked in sequence of return. The first "
            "return will have a Return Number of one, the second a Return "
            "Number of two, and so on up to five returns.";
    return_no.setDescription(text.str());
    return_no.isNumeric(true);
    return_no.isInteger(true);

    addField(return_no);
    text.str("");
    
    Field no_returns(FI_NumberOfReturns, "Number of Returns", DT_BIT, 4);
    text << "Number of Returns (for this emitted pulse): The Number of Returns "
            "is the total number of returns for a given pulse. For example, "
            "a laser data point may be return two (Return Number) within a "
            "total number of five returns.";
    no_returns.setDescription(text.str());
    no_returns.isNumeric(true);
    no_returns.isInteger(true);
    addField(no_returns);
    text.str("");

	Field class_flags(FI_ClassificationFlags, "Classification Flags", DT_BIT, 6);
	text << "Number of Returns (for this emitted pulse): The Number of Returns "
		"is the total number of returns for a given pulse. For example, "
		"a laser data point may be return two (Return Number) within a "
		"total number of five returns.";
	class_flags.setDescription(text.str());
	class_flags.isNumeric(true);
	class_flags.isInteger(true);
	addField(class_flags);
	text.str("");

    Field scan_dir(FI_ScanDirectionFlag, "Scan Direction", DT_BIT, 1);
    text << "The Scan Direction Flag denotes the direction at which the "
            "scanner mirror was traveling at the time of the output pulse. "
            "A bit value of 1 is a positive scan direction, and a bit value "
            "of 0 is a negative scan direction (where positive scan direction "
            "is a scan moving from the left side of the in-track direction to "
            "the right side and negative the opposite). ";
    scan_dir.setDescription(text.str());
    scan_dir.isNumeric(true);
    scan_dir.isInteger(true);
    addField(scan_dir);
    text.str("");
    
    Field edge(FI_EdgeOfFlightLine, "Flightline Edge", DT_BIT, 1);
    text << "The Edge of Flight Line data bit has a value of 1 only when "
            "the point is at the end of a scan. It is the last point on "
            "a given scan line before it changes direction.";
    edge.setDescription(text.str());
    edge.isNumeric(true);
    edge.isInteger(true);
    addField(edge);
    text.str("");

	Field scanner_channel(FI_ScannerChannel, "Scanner Channel", DT_UCHAR, 8);
	text << "Number of Returns (for this emitted pulse): The Number of Returns "
		"is the total number of returns for a given pulse. For example, "
		"a laser data point may be return two (Return Number) within a "
		"total number of five returns.";
	scanner_channel.setDescription(text.str());
	scanner_channel.isNumeric(true);
	scanner_channel.isInteger(true);
	scanner_channel.isSigned(false);
	addField(scanner_channel);
	text.str("");

    Field classification(FI_Classification, "Classification", DT_UCHAR, 8);
    text << "Classification in LAS 1.0 was essentially user defined and optional. "
            "LAS 1.1 defines a standard set of ASPRS classifications. In addition, "
            "the field is now mandatory. If a point has never been classified, this "
            "byte must be set to zero. There are no user defined classes since "
            "both point format 0 and point format 1 supply 8 bits per point for "
            "user defined operations. Note that the format for classification is a "
            "bit encoded field with the lower five bits used for class and the "
            "three high bits used for flags.";
    classification.setDescription(text.str());
    addField(classification);
    text.str("");
    
    Field scan_angle(FI_ScanAngleRank, "Scan Angle Rank", DT_UCHAR, 8);
    text << "The Scan Angle Rank is a signed one-byte number with a "
            "valid range from -90 to +90. The Scan Angle Rank is the "
            "angle (rounded to the nearest integer in the absolute "
            "value sense) at which the laser point was output from the "
            "laser system including the roll of the aircraft. The scan "
            "angle is within 1 degree of accuracy from +90 to -90 degrees. "
            "The scan angle is an angle based on 0 degrees being nadir, "
            "and -90 degrees to the left side of the aircraft in the "
            "direction of flight.";
    scan_angle.setDescription(text.str());
    scan_angle.isSigned(true);
    scan_angle.isInteger(true);
    scan_angle.isNumeric(true);
    addField(scan_angle);
    text.str("");
    
    Field point_source_id(FI_PointSourceID, "Point Source ID", DT_USHORT, 16);
    text << "This value indicates the file from which this point originated. "
            "Valid values for this field are 1 to 65,535 inclusive with zero "
            "being used for a special case discussed below. The numerical value "
            "corresponds to the File Source ID from which this point originated. "
            "Zero is reserved as a convenience to system implementers. A Point "
            "Source ID of zero implies that this point originated in this file. "
            "This implies that processing software should set the Point Source "
            "ID equal to the File Source ID of the file containing this point "
            "at some time during processing. ";
    point_source_id.setDescription(text.str());
    point_source_id.isInteger(true);
    point_source_id.isNumeric(true);
    addField(point_source_id);    
    text.str("");

    index_by_position& positionIndex = _index.get<position>();
    for (index_by_position::iterator i = positionIndex.begin(); i!= positionIndex.end(); ++i)
    {
        positionIndex.modify(i, SetRequired(true));
        positionIndex.modify(i, SetActive(true));
    }

}

void Schema::addColor()
{
    std::ostringstream text;
    
    Field red(FI_Red, "Red", DT_USHORT, 16);
    text << "The red image channel value associated with this point";
    red.setDescription(text.str()); 
    red.isRequired(true);
    red.isActive(true);
    red.isInteger(true);
    red.isNumeric(true);

    addField(red);
    text.str("");

    Field green(FI_Green, "Green", DT_USHORT, 16);
    text << "The green image channel value associated with this point";
    green.setDescription(text.str()); 
    green.isRequired(true);
    green.isActive(true);
    green.isInteger(true);
    green.isNumeric(true);

    addField(green);
    text.str("");

    Field blue(FI_Blue, "Blue", DT_USHORT, 16);
    text << "The blue image channel value associated with this point";
    blue.setDescription(text.str()); 
    blue.isRequired(true);
    blue.isActive(true);
    blue.isInteger(true);
    blue.isNumeric(true);

    addField(blue);
    text.str("");    
}


void Schema::addNIR()
{
	std::ostringstream text;

	Field nir(FI_NIR, "NIR", DT_USHORT, 16);
	text << "The near infrared image channel value associated with this point";
	nir.setDescription(text.str());
	nir.isRequired(true);
	nir.isActive(true);
	nir.isInteger(true);
	nir.isNumeric(true);

	addField(nir);
	text.str("");
}

void Schema::addTime()
{
    std::ostringstream text;
    
    Field t(FI_GNSSTime, "Time", DT_DOUBLE, 64);
    text << "The GNSS Time is the double floating point time tag value at "
            "which the point was acquired. It is GNSS Week Time if the "
            "Global Encoding low bit is clear and Adjusted Standard GNSS "
            "Time if the Global Encoding low bit is set.";
    t.setDescription(text.str()); 
    t.isRequired(true);
    t.isActive(true);
    t.isNumeric(true);

    addField(t);
    text.str("");    
}

void Schema::updateRequiredFields(PointFormat pointFormat)
{
    FieldArray user_dims;

    index_by_position& positionIndex = _index.get<position>();
    if (!positionIndex.empty())
    {
        // Keep any non-required fields the user may have added
        // and add them back to the list of fields
        for (index_by_position::const_iterator iter = positionIndex.begin(); iter != positionIndex.end(); ++iter)
        {
            if ( iter->isRequired() == false)
                user_dims.push_back(*iter);
        }
    }    
    // Sort the user fields so we preserve the order they were 
    // added in.
    std::sort(user_dims.begin(), user_dims.end(), sortFields);
    
    positionIndex.clear();
    
    // Reset the position counter.  Fields will be added in the 
    // order they need to be according to addASPRSBasicLasFields, etc.
    _nextPosition = 0; 

    switch (pointFormat) 
    {
        case PF_PointFormat0:
            addXYZ();
            break;
		case PF_PointFormat1:
			// Add the base fields
			addXYZ();
			addASPRSBasicLasFields();
			break;
        case PF_PointFormat2:
            addXYZ();
            addASPRSBasicLasFields();
            addTime();
            break;
        case PF_PointFormat3:
            addXYZ();
            addASPRSBasicLasFields();
            addColor();
            break;
        case PF_PointFormat4:
            // Add the base fields
            addXYZ();
            addASPRSBasicLasFields();
            addTime();
            addColor();
            break;
        case PF_PointFormat5:
            // Add the base fields
            addXYZ();
            addASPRSBasicLasFields();
            addTime();
            addWaveform();
            break;
        case PF_PointFormat6:
            // Add the base fields
            addXYZ();
            addASPRSBasicLasFields();
			addTime();
			addColor();
			addWaveform();
			break;
		case PF_PointFormat7:
			// Add the base fields
			addXYZ();
			addASPRSBasicLasFields();
			addTime();
			addColor();
			addNIR();
			break;
		case PF_PointFormat8:
			// Add the base fields
			addXYZ();
			addASPRSBasicLasFields();
			addTime();
			addColor();
			addNIR();
			addWaveform();
			break;
        case PF_PointFormatNone:
            break;
        case PF_PointFormatCustom:
            addXYZ();
            break;         
        default:
            std::ostringstream oss;
            oss << "Unhandled PointFormat id " << static_cast<uint32_t>(pointFormat);
            throw std::runtime_error(oss.str());
    }

    // Copy any user-created fields that are not 
    // required by the PointFormatName
    for (FieldArray::const_iterator j = user_dims.begin(); j != user_dims.end(); ++j)
    {
        // We need to use AddField to ensure that the sizes
        // and position offsets are updated.
        addField(*j);
    }

    calculateSizes();
}

bool Schema::isCustom() const
{
    // A custom schema has no fields that are required by the PointFormat
    // This must mean a user has added them themselves.  We only write 
    // schema definitions to files that have custom schemas.
    
    index_by_position const& positionIndex = _index.get<position>();
    index_by_position::const_iterator i;
    
    // return true; // For now, we'll always say we're  custom
    for (i = positionIndex.begin(); i != positionIndex.end(); ++i)
    {
        if ( i->isRequired() == false)
            return true;
    }
    return false;
}


void Schema::calculateSizes() 
{
    // Loop through the fields and update the bit and byte offset
    // values for each.  Additionally, update _bitSize and _baseBitSize
    // for the entire schema.  
    _bitSize = 0;
    _baseBitSize = 0;

    index_by_position& positionIndex = _index.get<position>();
    
    std::size_t byte_offset = 0;
    std::size_t bit_offset = 0;

    for (index_by_position::iterator i = positionIndex.begin(); i != positionIndex.end(); i++)
    {
        Field t = (*i);
        _bitSize += t.getBitSize(); 

        bit_offset = bit_offset + (t.getBitSize() % 8);

        t.setByteOffset(byte_offset);
        t.setBitOffset(bit_offset);
        positionIndex.replace(i, t);
        
        // We don't increment if this field is within the current byte
        if ( bit_offset % 8 == 0)
        {
            bit_offset = 0;
            byte_offset = byte_offset + t.getByteSize();
        }

        if ( t.isRequired() == true)
            _baseBitSize += t.getBitSize();        
    }

}

std::size_t Schema::getBaseByteSize() const
{
    return _baseBitSize / 8;
}

std::size_t Schema::getBitSize() const
{   
    return _bitSize;
}

void Schema::setDataFormat(PointFormat const& value)
{
    updateRequiredFields(value);
    _pointFormat = value;
}

std::size_t Schema::getByteSize() const
{
    
    return getBitSize() / 8;
}

void Schema::addField(Field const& field)
{
    // Increment the position;
    Field d(field);
    d.setPosition(_nextPosition);
    _nextPosition++;
    
    // Add the field ptr on the fields map
    index_by_index & idx = _index.get<index>();
	idx.push_back(d);   

    // Update all of our sizes
    calculateSizes();
}

bool Schema::removeField(size_t ind)
{
	index_by_index& idx = _index.get<index>();
	if (ind <= idx.size())
	{
		idx.remove(idx.at(ind));
		return true;
	}
	else
		return false;

	calculateSizes();
	return true;
}

bool Schema::removeAllFields()
{
	size_t count = getFieldCount();
	for (auto i = 0; i < count; i++)
	{
		if (removeField(0) == false)
			return false;
	}

	return true;
}

bool Schema::getFields(std::string const& aName, FieldArray &fields) const
{    
    index_by_name::const_iterator it = _index.get<name>().find(aName);

	if (it == _index.get<name>().end())
		return false;

    while (it != _index.get<name>().end())
    {
        fields.push_back(*it);
		it++;
    }    
    
    return true;
}

bool Schema::getField(size_t ind, Field &field) const
{
    index_by_index const& idx = _index.get<index>();
    
    if (ind <= idx.size())
    {
		field = idx.at(ind);
        return true;
    }else 
        return false;
}

bool Schema::hasField(FieldId id) const
{
	return getFieldCount() > 0;
}

size_t Schema::getFieldCount() const
{
    index_by_index const& idx = _index.get<index>();
    
    return idx.size();
}

Field* Schema::getFieldById(FieldId aId)
{
	index_by_id const& id_index = _index.get<id>();
	index_by_id::const_iterator it0, it1;
	boost::tuples::tie(it0, it1) = id_index.equal_range(aId);

	if (it0 == it1)
		return NULL;
	else
	 return (Field *)&(*it0);
}

bool Schema::getFieldsById(FieldId aId, FieldArray &fields) const
{
    index_by_id const& id_index = _index.get<id>();
    index_by_id::const_iterator it0, it1;
    boost::tuples::tie(it0, it1) = id_index.equal_range(aId);
    
    if (it0 == it1)
        return false;

    while(it0 != it1)
    {
      fields.push_back(*it0);
      ++it0;
    }

    return true;   
}

std::vector<std::string> Schema::getFieldNames() const
{
    std::vector<std::string> output;

    index_by_position const& positionIndex = _index.get<position>();
    index_by_position::const_iterator it = positionIndex.begin();
    
    while (it != positionIndex.end()) {
        output.push_back(it->getName());
        it++;
    }

    return output;
}

size_t Schema::getBandCount() const
{
	return getFieldCountById(FI_BandValue);
}

size_t Schema::getFieldCountById(FieldId aId) const
{
	index_by_id const& id_index = _index.get<id>();
	index_by_id::const_iterator it0, it1;
	boost::tuples::tie(it0, it1) = id_index.equal_range(aId);

	if (it0 == it1)
		return 0;

	size_t count = 0;
	while (it0 != it1)
	{
		++it0;
		++count;
	}

	return count;
}

bool Schema::getBand(size_t n, Band &band) const
{
	size_t index;
	if (getNthIndex(FI_BandValue, n, index) && getField(index, band))
		return true;
	
	return false;
}

bool Schema::getBandDesc(size_t n, BandDesc& bandDesc) const
{
	Band band;
	size_t index;
	if (getNthIndex(FI_BandValue, n, index) && getField(index, band))
	{
		bandDesc.type = band.getDataType();
		bandDesc.name = band.getName();
		bandDesc.description = band.getDescription();
		return true;
	}

	return false;
}

bool Schema::getBandDescs(BandDescArray& bandDescs) const
{
	size_t count = getBandCount();
	bandDescs.clear();
	BandDesc bandDesc;
	for (auto i = 0; i < count; i++)
	{
		if (getBandDesc(i, bandDesc))
			bandDescs.push_back(bandDesc);
		else
			return false;
	}

	return true;
}

bool Schema::addBands(const BandDesc &band, size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		if (!addBand(band.type, band.name, band.description))
			return false;
	}

	return true;
}

bool Schema::addBands(const BandDescArray &bands)
{
	for (size_t i = 0; i < bands.size(); i++)
	{
		if (!addBand(bands[i].type, bands[i].name, bands[i].description))
			return false;
	}

	return true;
}

bool Schema::removeBand(size_t index)
{
	size_t ind;

	// found the band
	if (getNthIndex(FI_BandValue, index, ind))
	{
		// remove the field
		return removeField(ind);
	}

	// if no band is found
	return true;
}

bool Schema::removeAllBands()
{
	size_t count = getBandCount();
	for (auto i = 0; i < count; i++)
	{
		if (removeBand(0) == false)
			return false;
	}

	return true;
}

bool Schema::addBand(DataType type, const std::string &name, const std::string &description)
{
	size_t bits = 0;

	switch (type)
	{
	case DT_UCHAR:
		bits = 8;
		break;
	case DT_SHORT:
	case DT_USHORT:
		bits = 16;
		break;
	case DT_LONG:
	case DT_ULONG:
	case DT_FLOAT:
		bits = 32;
		break;
	case DT_LONGLONG:
	case DT_ULONGLONG:
	case DT_DOUBLE:
		bits = 64;
		break;
	default:
		throw hsl::invalid_band_datatype("unsupported data type.");
		return false;
	}

	std::ostringstream text;
	hsl::Field band(hsl::FI_BandValue, name, type, bits);
	text << "The multispectral/hyperspectral band values for each spectral band.";
	band.setDescription(text.str());
	band.isRequired(false);
	band.isActive(true);
	band.isNumeric(true);
	band.isScaled(false);
	band.isOffseted(false);
	addField(band);

	return true;
}

bool Schema::getNthIndex(FieldId id, size_t n, size_t &ind) const
{
	index_by_index const & idx = _index.get<index>();
	index_by_index::const_iterator it = idx.begin();
    if (_index.size() < n)
        return false;
        
    size_t i = 0;
    size_t count = 0;

    while (it != idx.end()) {
        if (it->getId() == id)
        {
            if (i == n)
            {
                ind = count;
                return true;
            }
            i++;
        }

        it++;
        count++;
    }

    return false;
}

bool Schema::operator==(const Schema& input) const
{

    index_by_index const& current = _index.get<index>();
    index_by_index const& other = input._index.get<index>();
    
    
    for (uint32_t i = 0; i!= current.size(); ++i)
    {
        if (other[i] != current[i]) return false;
    }
    
    return true;

}

bool Schema::addFieldFromFieldDesc(FieldId fieldId, const FieldDesc &fieldDesc)
{
	FieldId id = fieldId;
	std::string name;
	std::size_t size_in_bits;
	std::string description;
	bool isNumeric = false;
	bool isInteger = false;
	bool isSigned = false;
	bool isScaled = false;
	bool isOffseted = false;

	DataType type = fieldDesc.type;
	switch (type)
	{
	case DT_BIT:
	{
		BitField f = fieldDesc.data.bitField;
		name = f.name;
		size_in_bits = f.size_in_bits;
		description = f.description;
		Field d(id, name, DT_BIT, size_in_bits);
		d.setDescription(description);
		d.isNumeric(isNumeric);
		d.isInteger(isInteger);
		d.isSigned(isSigned);
		d.isScaled(isScaled);
		d.isOffseted(isOffseted);
		addField(d);
	}
	break;
	case DT_CHAR:
	{
		CharField f = fieldDesc.data.charField;
		name = f.name;
		size_in_bits = f.size_in_bits;
		if (size_in_bits % 8 != 0)
			return false;
		description = f.description;
		isSigned = true;
		Field d(id, name, DT_CHAR, size_in_bits);
		d.setDescription(description);
		d.isNumeric(isNumeric);
		d.isInteger(isInteger);
		d.isSigned(isSigned);
		d.isScaled(isScaled);
		d.isOffseted(isOffseted);
		addField(d);
	}
	break;
	case DT_UCHAR:
	{
		UCharField f = fieldDesc.data.ucharField;
		name = f.name;
		size_in_bits = f.size_in_bits;
		if ((size_in_bits / 8.0) > sizeof(uint8_t))
			return false;
		description = f.description;
		isInteger = true;
		isNumeric = true;
		isSigned = false;
		isScaled = true;
		isOffseted = true;
		Field d(id, name, DT_UCHAR, size_in_bits);
		d.setDescription(description);
		d.isNumeric(isNumeric);
		d.isInteger(isInteger);
		d.isSigned(isSigned);
		if (f.options.bits.max_bit & 0x01)
			d.setMaximum(f.max);
		if (f.options.bits.min_bit & 0x01)
			d.setMinimum(f.min);
		if (f.options.bits.scale_bit & 0x01)
		{
			d.isScaled(isScaled);
			d.setScale(f.scale);
		}
		else{
			d.isScaled(false);
		}
		if (f.options.bits.offset_bit & 0x01)
		{
			d.isOffseted(isOffseted);
			d.setOffset(f.offset);
		}
		else{
			d.isOffseted(false);
		}

		addField(d);
	}
	break;
	case DT_SHORT:
	{
		ShortField f = fieldDesc.data.shortField;
		name = f.name;
		if (f.options.bits.size_in_bits_bit & 0x01)
			size_in_bits = f.size_in_bits;
		else
			size_in_bits = sizeof(int16_t) * 8;
		if ((size_in_bits / 8.0) > sizeof(int16_t))
			return false;
		description = f.description;
		isInteger = true;
		isNumeric = true;
		isSigned = true;
		isScaled = true;
		isOffseted = true;
		Field d(id, name, DT_SHORT, size_in_bits);
		d.setDescription(description);
		d.isNumeric(isNumeric);
		d.isInteger(isInteger);
		d.isSigned(isSigned);
		if (f.options.bits.max_bit & 0x01)
			d.setMaximum(f.max);
		if (f.options.bits.min_bit & 0x01)
			d.setMinimum(f.min);
		if (f.options.bits.scale_bit & 0x01)
		{
			d.isScaled(isScaled);
			d.setScale(f.scale);
		}
		else{
			d.isScaled(false);
		}
		if (f.options.bits.offset_bit & 0x01)
		{
			d.isOffseted(isOffseted);
			d.setOffset(f.offset);
		}
		else{
			d.isOffseted(false);
		}

		addField(d);
	}
	break;
	case DT_USHORT:
	{
		UShortField f = fieldDesc.data.ushortField;
		name = f.name;
		if (f.options.bits.size_in_bits_bit & 0x01)
			size_in_bits = f.size_in_bits;
		else
			size_in_bits = sizeof(unsigned short) * 8;
		if ((size_in_bits / 8.0) > sizeof(unsigned short))
			return false;
		description = f.description;
		isInteger = true;
		isNumeric = true;
		isSigned = false;
		isScaled = true;
		isOffseted = true;
		Field d(id, name, DT_USHORT, size_in_bits);
		d.setDescription(description);
		d.isNumeric(isNumeric);
		d.isInteger(isInteger);
		d.isSigned(isSigned);
		if (f.options.bits.max_bit & 0x01)
			d.setMaximum(f.max);
		if (f.options.bits.min_bit & 0x01)
			d.setMinimum(f.min);
		if (f.options.bits.scale_bit & 0x01)
		{
			d.isScaled(isScaled);
			d.setScale(f.scale);
		}
		else{
			d.isScaled(false);
		}
		if (f.options.bits.offset_bit & 0x01)
		{
			d.isOffseted(isOffseted);
			d.setOffset(f.offset);
		}
		else{
			d.isOffseted(false);
		}

		addField(d);
	}
	break;
	case DT_LONG:
	{
		LongField f = fieldDesc.data.longField;
		name = f.name;
		if (f.options.bits.size_in_bits_bit & 0x01)
			size_in_bits = f.size_in_bits;
		else
			size_in_bits = sizeof(long) * 8;
		if ((size_in_bits / 8.0) > sizeof(long))
			return false;
		description = f.description;
		isInteger = true;
		isNumeric = true;
		isSigned = true;
		isScaled = true;
		isOffseted = true;
		Field d(id, name, DT_LONG, size_in_bits);
		d.setDescription(description);
		d.isNumeric(isNumeric);
		d.isInteger(isInteger);
		d.isSigned(isSigned);
		if (f.options.bits.max_bit & 0x01)
			d.setMaximum(f.max);
		if (f.options.bits.min_bit & 0x01)
			d.setMinimum(f.min);
		if (f.options.bits.scale_bit & 0x01)
		{
			d.isScaled(isScaled);
			d.setScale(f.scale);
		}
		else{
			d.isScaled(false);
		}
		if (f.options.bits.offset_bit & 0x01)
		{
			d.isOffseted(isOffseted);
			d.setOffset(f.offset);
		}
		else{
			d.isOffseted(false);
		}

		addField(d);
	}
	break;
	case DT_ULONG:
	{
		ULongField f = fieldDesc.data.ulongField;
		name = f.name;
		if (f.options.bits.size_in_bits_bit & 0x01)
			size_in_bits = f.size_in_bits;
		else
			size_in_bits = sizeof(unsigned long) * 8;
		if ((size_in_bits / 8.0) > sizeof(unsigned long))
			return false;
		description = f.description;
		isInteger = true;
		isNumeric = true;
		isSigned = false;
		isScaled = true;
		isOffseted = true;
		Field d(id, name, DT_ULONG, size_in_bits);
		d.setDescription(description);
		d.isNumeric(isNumeric);
		d.isInteger(isInteger);
		d.isSigned(isSigned);
		if (f.options.bits.max_bit & 0x01)
			d.setMaximum(f.max);
		if (f.options.bits.min_bit & 0x01)
			d.setMinimum(f.min);
		if (f.options.bits.scale_bit & 0x01)
		{
			d.isScaled(isScaled);
			d.setScale(f.scale);
		}
		else{
			d.isScaled(false);
		}
		if (f.options.bits.offset_bit & 0x01)
		{
			d.isOffseted(isOffseted);
			d.setOffset(f.offset);
		}
		else{
			d.isOffseted(false);
		}

		addField(d);
	}
	break;
	case DT_LONGLONG:
	{
		LongLongField f = fieldDesc.data.longlongField;
		name = f.name;
		if (f.options.bits.size_in_bits_bit & 0x01)
			size_in_bits = f.size_in_bits;
		else
			size_in_bits = sizeof(long long) * 8;
		if ((size_in_bits / 8.0) > sizeof(long long))
			return false;
		description = f.description;
		isInteger = true;
		isNumeric = true;
		isSigned = true;
		isScaled = true;
		isOffseted = true;
		Field d(id, name, DT_LONGLONG, size_in_bits);
		d.setDescription(description);
		d.isNumeric(isNumeric);
		d.isInteger(isInteger);
		d.isSigned(isSigned);
		if (f.options.bits.max_bit & 0x01)
			d.setMaximum(f.max);
		if (f.options.bits.min_bit & 0x01)
			d.setMinimum(f.min);
		if (f.options.bits.scale_bit & 001)
		{
			d.isScaled(isScaled);
			d.setScale(f.scale);
		}
		else{
			d.isScaled(false);
		}
		if (f.options.bits.offset_bit & 0x01)
		{
			d.isOffseted(isOffseted);
			d.setOffset(f.offset);
		}
		else{
			d.isOffseted(false);
		}

		addField(d);
	}
	break;
	case DT_ULONGLONG:
	{
		ULongLongField f = fieldDesc.data.ulonglongField;
		name = f.name;
		if (f.options.bits.size_in_bits_bit & 0x01)
			size_in_bits = f.size_in_bits;
		else
			size_in_bits = sizeof(unsigned long long) * 8;
		if ((size_in_bits / 8.0) > sizeof(unsigned long long))
			return false;
		description = f.description;
		isInteger = true;
		isNumeric = true;
		isSigned = false;
		isScaled = true;
		isOffseted = true;
		Field d(id, name, DT_ULONGLONG, size_in_bits);
		d.setDescription(description);
		d.isNumeric(isNumeric);
		d.isInteger(isInteger);
		d.isSigned(isSigned);
		if (f.options.bits.max_bit & 0x01)
			d.setMaximum(f.max);
		if (f.options.bits.min_bit & 0x01)
			d.setMinimum(f.min);
		if (f.options.bits.scale_bit & 0x01)
		{
			d.isScaled(isScaled);
			d.setScale(f.scale);
		}
		else{
			d.isScaled(false);
		}
		if (f.options.bits.offset_bit & 0x01)
		{
			d.isOffseted(isOffseted);
			d.setOffset(f.offset);
		}
		else{
			d.isOffseted(false);
		}

		addField(d);
	}
	break;
	case DT_FLOAT:
	{
		FloatField f = fieldDesc.data.floatField;
		name = f.name;
		size_in_bits = sizeof(float) * 8;
		description = f.description;
		isInteger = false;
		isNumeric = true;
		isSigned = true;
		isScaled = true;
		isOffseted = true;
		Field d(id, name, DT_FLOAT, size_in_bits);
		d.setDescription(description);
		d.isNumeric(isNumeric);
		d.isInteger(isInteger);
		d.isSigned(isSigned);
		if (f.options.bits.max_bit & 0x01)
			d.setMaximum(f.max);
		if (f.options.bits.min_bit & 0x01)
			d.setMinimum(f.min);
		if (f.options.bits.scale_bit & 0x01)
		{
			d.isScaled(isScaled);
			d.setScale(f.scale);
		}
		else{
			d.isScaled(false);
		}
		if (f.options.bits.offset_bit & 0x01)
		{
			d.isOffseted(isOffseted);
			d.setOffset(f.offset);
		}
		else{
			d.isOffseted(false);
		}

		addField(d);
	}
	break;
	case DT_DOUBLE:
	{
		DoubleField f = fieldDesc.data.doubleField;
		name = f.name;
		size_in_bits = sizeof(double) * 8;
		description = f.description;
		isInteger = false;
		isNumeric = true;
		isSigned = true;
		isScaled = true;
		isOffseted = true;
		Field d(id, name, DT_DOUBLE, size_in_bits);
		d.setDescription(description);
		d.isNumeric(isNumeric);
		d.isInteger(isInteger);
		d.isSigned(isSigned);
		if (f.options.bits.max_bit & 0x01)
			d.setMaximum(f.max);
		if (f.options.bits.min_bit & 0x01)
			d.setMinimum(f.min);
		if (f.options.bits.scale_bit & 0x01)
		{
			d.isScaled(isScaled);
			d.setScale(f.scale);
		}
		else{
			d.isScaled(false);
		}
		if (f.options.bits.offset_bit & 0x01)
		{
			d.isOffseted(isOffseted);
			d.setOffset(f.offset);
		}
		else{
			d.isOffseted(false);
		}

		addField(d);
	}
	break;
	default:
		return false;
		break;
	}
	return true;
}

bool Schema::getFieldDescFromField(const Field &field, FieldDesc &fieldDesc)
{
	fieldDesc.type = field.getDataType();

	switch (fieldDesc.type)
	{
	case DT_BIT:
	{
		BitField &f = fieldDesc.data.bitField;
		std::strncpy(f.name, field.getName().c_str(), sizeof(f.name));
		f.data_type = DT_BIT;
		f.size_in_bits = field.getBitSize();
		f.options.bits.size_in_bits_bit |= 0x01;
		std::strncpy(f.description, field.getDescription().c_str(), sizeof(f.description));
		f.min = 0;
		f.max = 1;
		f.scale = 1;
		f.offset = 0;
		f.options.c = 0;
	}
	break;
	case DT_CHAR:
	{
		CharField &f = fieldDesc.data.charField;
		std::strncpy(f.name, field.getName().c_str(), sizeof(f.name));
		f.data_type = DT_CHAR;
		f.size_in_bits = field.getBitSize();
		f.options.bits.size_in_bits_bit |= 0x01;
		std::strncpy(f.description, field.getDescription().c_str(), sizeof(f.description));
		f.options.c = 0;
	}
	break;
	case DT_UCHAR:
	{
		UCharField &f = fieldDesc.data.ucharField;
		std::strncpy(f.name, field.getName().c_str(), sizeof(f.name));
		f.data_type = DT_UCHAR;
		f.size_in_bits = field.getBitSize();
		f.options.bits.size_in_bits_bit |= 0x01;
		std::strncpy(f.description, field.getDescription().c_str(), sizeof(f.description));
		f.options.c = 0;
	}
	break;
	case DT_SHORT:
	{
		ShortField &f = fieldDesc.data.shortField;
		std::strncpy(f.name, field.getName().c_str(), sizeof(f.name));
		f.data_type = DT_SHORT;
		f.options.c = 0;
		f.options.bits.size_in_bits_bit = 1;
		f.options.bits.scale_bit = field.isScaled();
		f.options.bits.offset_bit = field.isOffseted();
		f.size_in_bits = field.getBitSize();
		std::strncpy(f.description, field.getDescription().c_str(), sizeof(f.description));
		f.min = (int16_t)field.getMinimum();
		f.max = (int16_t)field.getMaximum();
		if (field.isScaled())
			f.scale = field.getScale();
		if (field.isOffseted())
			f.offset = field.getOffset();
	}
	break;
	case DT_USHORT:
	{
		UShortField &f = fieldDesc.data.ushortField;
		std::strncpy(f.name, field.getName().c_str(), sizeof(f.name));
		f.data_type = DT_USHORT;
		f.options.c = 0;
		f.options.bits.size_in_bits_bit = 1;
		f.options.bits.scale_bit = field.isScaled();
		f.options.bits.offset_bit = field.isOffseted();
		f.size_in_bits = field.getBitSize();
		std::strncpy(f.description, field.getDescription().c_str(), sizeof(f.description));
		f.min = (uint16_t)field.getMinimum();
		f.max = (uint16_t)field.getMaximum();
		if (field.isScaled())
			f.scale = field.getScale();
		if (field.isOffseted())
			f.offset = field.getOffset();
	}
	break;
	case DT_LONG:
	{
		LongField &f = fieldDesc.data.longField;
		std::strncpy(f.name, field.getName().c_str(), sizeof(f.name));
		f.data_type = DT_LONG;
		f.options.c = 0;
		f.options.bits.size_in_bits_bit = 1;
		f.options.bits.scale_bit = field.isScaled();
		f.options.bits.offset_bit = field.isOffseted();
		f.size_in_bits = field.getBitSize();
		std::strncpy(f.description, field.getDescription().c_str(), sizeof(f.description));
		f.min = (int32_t)field.getMinimum();
		f.max = (int32_t)field.getMaximum();
		if (field.isScaled())
			f.scale = field.getScale();
		if (field.isOffseted())
			f.offset = field.getOffset();
	}
	break;
	case DT_ULONG:
	{
		ULongField &f = fieldDesc.data.ulongField;
		std::strncpy(f.name, field.getName().c_str(), sizeof(f.name));
		f.data_type = DT_ULONG;
		f.options.c = 0;
		f.options.bits.size_in_bits_bit = 1;
		f.options.bits.scale_bit = field.isScaled();
		f.options.bits.offset_bit = field.isOffseted();
		f.size_in_bits = field.getBitSize();
		std::strncpy(f.description, field.getDescription().c_str(), sizeof(f.description));
		f.min = (uint32_t)field.getMinimum();
		f.max = (uint32_t)field.getMaximum();
		if (field.isScaled())
			f.scale = field.getScale();
		if (field.isOffseted())
			f.offset = field.getOffset();
	}
	break;
	case DT_LONGLONG:
	{
		LongLongField &f = fieldDesc.data.longlongField;
		std::strncpy(f.name, field.getName().c_str(), sizeof(f.name));
		f.data_type = DT_LONGLONG;
		f.options.c = 0;
		f.options.bits.size_in_bits_bit = 1;
		f.options.bits.scale_bit = field.isScaled();
		f.options.bits.offset_bit = field.isOffseted();
		f.size_in_bits = field.getBitSize();
		std::strncpy(f.description, field.getDescription().c_str(), sizeof(f.description));
		f.min = (int64_t)field.getMinimum();
		f.max = (int64_t)field.getMaximum();
		if (field.isScaled())
			f.scale = field.getScale();
		if (field.isOffseted())
			f.offset = field.getOffset();
	}
	break;
	case DT_ULONGLONG:
	{
		ULongLongField &f = fieldDesc.data.ulonglongField;
		std::strncpy(f.name, field.getName().c_str(), sizeof(f.name));
		f.data_type = DT_ULONGLONG;
		f.options.bits.size_in_bits_bit = 1;
		f.options.bits.scale_bit = field.isScaled();
		f.options.bits.offset_bit = field.isOffseted();
		f.size_in_bits = field.getBitSize();
		std::strncpy(f.description, field.getDescription().c_str(), sizeof(f.description));
		f.min = (uint64_t)field.getMinimum();
		f.max = (uint64_t)field.getMaximum();
		if (field.isScaled())
			f.scale = field.getScale();
		if (field.isOffseted())
			f.offset = field.getOffset();
	}
	break;
	case DT_FLOAT:
	{
		FloatField &f = fieldDesc.data.floatField;
		std::strncpy(f.name, field.getName().c_str(), sizeof(f.name));
		f.data_type = DT_FLOAT;
		f.options.bits.size_in_bits_bit = 0;
		f.options.bits.scale_bit = field.isScaled();
		f.options.bits.offset_bit = field.isOffseted();
		f.size_in_bits = field.getBitSize();
		std::strncpy(f.description, field.getDescription().c_str(), sizeof(f.description));
		f.min = (float)field.getMinimum();
		f.max = (float)field.getMaximum();
		if (field.isScaled())
			f.scale = field.getScale();
		if (field.isOffseted())
			f.offset = field.getOffset();
	}
	break;
	case DT_DOUBLE:
	{
		DoubleField &f = fieldDesc.data.doubleField;
		std::strncpy(f.name, field.getName().c_str(), sizeof(f.name));
		f.data_type = DT_DOUBLE;
		f.options.bits.size_in_bits_bit = 0;
		f.options.bits.scale_bit = field.isScaled();
		f.options.bits.offset_bit = field.isOffseted();
		f.size_in_bits = field.getBitSize();
		std::strncpy(f.description, field.getDescription().c_str(), sizeof(f.description));
		f.min = field.getMinimum();
		f.max = field.getMaximum();
		if (field.isScaled())
			f.scale = field.getScale();
		if (field.isOffseted())
			f.offset = field.getOffset();
	}
	break;
	default:
		return false;
		break;
	}
	return true;
}

}
