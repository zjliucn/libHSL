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

#pragma once

#include "hslDefinitions.h"
#include "Field.h"

// boost
#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/array.hpp>
#include <boost/optional.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/random_access_index.hpp>

#include <iosfwd>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/unordered_map.hpp>

namespace hsl
{

class BandDesc
{
public:
	BandDesc(DataType theType, const std::string theName = "", const std::string theDescription = "") : type(theType), name(theName), description(theDescription) {};

public:
	DataType type;
	const std::string name;
	const std::string description;
};

typedef std::vector<Field> FieldArray;
typedef std::vector<BandDesc> BandDescArray;


struct name{};
struct position{};
struct index{};
struct id{};


typedef boost::multi_index::multi_index_container<
  Field,
  boost::multi_index::indexed_by<
    // sort by Field::operator<
    boost::multi_index::ordered_unique<boost::multi_index::tag<position>, boost::multi_index::identity<Field> >,
    
    // Random access
    boost::multi_index::random_access<boost::multi_index::tag<index> >,

    boost::multi_index::ordered_non_unique<boost::multi_index::tag<id>, boost::multi_index::const_mem_fun<Field, FieldId, &Field::getId> >,

    // sort by less<string> on getName
    boost::multi_index::hashed_non_unique<boost::multi_index::tag<name>, boost::multi_index::const_mem_fun<Field, std::string const&, &Field::getName> >
      >
> IndexMap;

typedef IndexMap::index<name>::type index_by_name;
typedef IndexMap::index<position>::type index_by_position;
typedef IndexMap::index<index>::type index_by_index;
typedef IndexMap::index<id>::type index_by_id;

/// Schema definition
class LIBHSL_API Schema
{
public:
    typedef std::vector<Field> FieldArray;
    
    // Schema();
    Schema(PointFormat format);
    Schema& operator=(Schema const& rhs);

    bool operator==(const Schema& other) const;
    bool operator!=(const Schema& other) const { return !(*this == other); }
        
    Schema(Schema const& other);
    
    ~Schema() {}

    /// Fetch byte size
    std::size_t getByteSize() const;

    std::size_t getBitSize() const;
    void calculateSizes();

    /// Get the base size (only accounting for Time, Color, etc )
    std::size_t getBaseByteSize() const;

    PointFormat getDataFormat() const { return _pointFormat; }
    void setDataFormat(PointFormat const& value);
    
    void addField(Field const& field);
	bool removeField(size_t index);

	bool getFields(std::string const& name, FieldArray &fields) const;
    bool getField(size_t index, Field &field) const;

	bool hasField(FieldId id) const;
    bool getFieldsById(FieldId id, FieldArray &fields) const;
	size_t getFieldCountById(FieldId id) const;

    bool getNthIndex(FieldId id, size_t n, size_t &index) const;
   
    std::vector<std::string> getFieldNames() const;
    IndexMap const& getFields() const { return _index; }
    size_t getFieldCount() const;

	size_t getBandCount() const;
	bool getBand(size_t n, Band &band) const;
	bool removeBand(size_t index);

	bool addBands(const BandDesc &band, size_t count);
	bool addBands(const BandDescArray &bands);

    uint16_t getSchemaVersion() const { return _schemaVersion; }
    void setSchemaVersion(uint16_t v) { _schemaVersion = v; }
    
    bool isCustom() const;

	bool addFieldFromFieldDesc(FieldId fieldId, const FieldDesc &fieldDesc);

	static bool getFieldDescFromField(const Field &field, FieldDesc &fieldDesc);

private:
    void addXYZ();
    void addTime();
    void addColor();
	void addNIR();
    void addWaveform();
    void addASPRSBasicLasFields();
    void updateRequiredFields(PointFormat pointFormat);

	bool addBand(DataType type, const std::string &name = "", const std::string &description = "");

protected:    
    PointFormat     _pointFormat;
    uint32_t        _nextPosition;
    std::size_t     _bitSize;
    std::size_t     _baseBitSize;
    uint16_t        _schemaVersion;
    
private:
    IndexMap        _index;
};

bool inline sortFields(Field i, Field j) 
{ 
    return i < j; 
}

} 