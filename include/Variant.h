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

#include <string>
#include <vector>
#include <type_traits>
#include <boost/any.hpp>
#include "hslDefinitions.h"
#include <boost/dynamic_bitset.hpp>
#include "Exception.h"


namespace hsl
{

class LIBHSL_API Variant
{
public:
	Variant();
	Variant(const boost::any &o);
	~Variant();

    Variant& operator=(Variant const& rhs);
	Variant(Variant const& other);

    bool operator==(const Variant& other) const;
    bool operator!=(const Variant& other) const { return !(*this == other); }

	boost::any getValue() const;
	void setValue(boost::any v){ _value = v; };

	void operator = (boost::any v){ _value = v; };

	bool isEmpty() const;

	DataType getType() const;	
	bool isType(DataType type) const;

	bool getValue(boost::dynamic_bitset<> &v) const;
	bool getValue(std::vector<uint8_t> &v) const;
	bool getValue(char * v) const;
	bool getValue(std::string &v) const;
	bool getValue(uint8_t &v) const;
	bool getValue(int16_t &v) const;
	bool getValue(uint16_t &v) const;
	bool getValue(int32_t &v) const;
	bool getValue(uint32_t &v) const;	
	bool getValue(int64_t &v) const;
	bool getValue(uint64_t &v) const;					
	bool getValue(float &v) const;
	bool getValue(double &v) const;	

	bool getByteSize(size_t &size) const;
	bool getBitSize(size_t &size) const;

private:
	boost::any _value;
};

typedef std::vector<Variant> VariantArray;


}
