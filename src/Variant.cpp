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


#include "Variant.h"
#include <string>
#include <boost/lexical_cast.hpp>
#include "Exception.h"

namespace hsl
{

Variant::Variant()
{
}

Variant::Variant(const boost::any &o) : _value(o)
{
}

Variant::~Variant(void)
{
}

/// copy constructor
Variant::Variant(Variant const& other) : _value(other._value)
{
}

/// assignment operator
Variant& Variant::operator=(Variant const& rhs)
{
    if (&rhs != this)
    {
        _value = rhs._value;
    }
    
    return *this;
}

bool Variant::operator==(const Variant& other) const
{
    if (&other == this) return true;
    if (_value.type() !=  other._value.type())
        return false;

    if ( _value.type() == typeid(std::string))
        return boost::any_cast<std::string>(_value) == boost::any_cast<std::string>( other._value);
    else if (other._value.type() == typeid(char *) || other._value.type() == typeid(const char *))
	{
		const char *str1 = boost::any_cast<const char *>(_value);
		const char *str2 = boost::any_cast<const char *>(_value);
		if (strcmp(str1, str2) == 0)
		{
        	return true;
		}else
		{
			return false;
		}		
	}else if ( other._value.type() == typeid(uint8_t))
        return boost::any_cast<uint8_t>(_value) == boost::any_cast<uint8_t>( other._value);
    else if ( other._value.type() == typeid(int16_t))
        return boost::any_cast<int16_t>(_value) == boost::any_cast<int16_t>( other._value);	
    else if ( other._value.type() == typeid(uint16_t))
        return boost::any_cast<uint16_t>(_value) == boost::any_cast<uint16_t>( other._value);	
    else if ( other._value.type() == typeid(int32_t))
        return boost::any_cast<int32_t>(_value) == boost::any_cast<int32_t>( other._value);
    else if ( other._value.type() == typeid(uint32_t))
        return boost::any_cast<uint32_t>(_value) == boost::any_cast<uint32_t>( other._value);	
    else if ( other._value.type() == typeid(int64_t))
        return boost::any_cast<int64_t>(_value) == boost::any_cast<int64_t>( other._value);	
    else if ( other._value.type() == typeid(uint64_t))
        return boost::any_cast<uint64_t>(_value) == boost::any_cast<uint64_t>( other._value);	
    else if ( other._value.type() == typeid(float))
        return boost::any_cast<float>(_value) == boost::any_cast<float>( other._value);	
    else if ( other._value.type() == typeid(double))
        return boost::any_cast<double>(_value) == boost::any_cast<double>( other._value);
	else
	{
		 throw std::runtime_error("comparison of any unimplemented for type");
	}	 

    return false;
}

boost::any Variant::getValue() const
{
	return _value;
}

bool Variant::isType(DataType type) const
{
	bool state = false;

	switch (type)
	{
	case DT_BIT:
		state = (_value.type() == typeid(boost::dynamic_bitset<>)) ? true : false;
		break;
	case DT_CHAR:
		state = (_value.type() == typeid(char *) || _value.type() == typeid(const char *)) ? true : false;
		break;
	case DT_UCHAR:
		state = (_value.type() == typeid(uint8_t)) ? true : false;
		break;
	case DT_SHORT:
		state = (_value.type() == typeid(int16_t)) ? true : false;
		break;
	case DT_USHORT:
		state = (_value.type() == typeid(uint16_t)) ? true : false;
		break;
	case DT_LONG:
		state = (_value.type() == typeid(int32_t)) ? true : false;
		break;
	case DT_ULONG:
		state = (_value.type() == typeid(uint32_t)) ? true : false;
		break;
	case DT_LONGLONG:
		state = (_value.type() == typeid(int64_t)) ? true : false;
		break;
	case DT_ULONGLONG:
		state = (_value.type() == typeid(uint64_t)) ? true : false;
		break;
	case DT_FLOAT:
		state = (_value.type() == typeid(float)) ? true : false;
		break;
	case DT_DOUBLE:
		state = (_value.type() == typeid(double)) ? true : false;
		break;
	default:
		state = false;
		break;
	}

	return state;
}

DataType Variant::getType() const
{
	if(_value.type() == typeid(boost::dynamic_bitset<>)) 
		return DT_BIT;
	else if (_value.type() == typeid(char *) || _value.type() == typeid(const char *))
	{
		return DT_CHAR;
	}else if (_value.type() == typeid(uint8_t))
	{
		return DT_UCHAR;
	}else if (_value.type() == typeid(int16_t)) 
	{
		return DT_SHORT; 
	}else if (_value.type() == typeid(uint16_t)) 
	{
		return DT_USHORT;
	}else if (_value.type() == typeid(int32_t))
	{
		return DT_LONG;
	}else if (_value.type() == typeid(uint32_t))
	{
		return DT_ULONG;
	}else if (_value.type() == typeid(int64_t))
	{
		return DT_LONGLONG;
	}else if (_value.type() == typeid(uint64_t))
	{
		return DT_ULONGLONG;
	}else if (_value.type() == typeid(float))
	{
		return DT_FLOAT;
	}else if (_value.type() == typeid(double))
	{
		return DT_DOUBLE;
	}else
	{
		return DT_UNKNOWN;
	}
}

bool Variant::getByteSize(size_t &size) const
{
	bool state = false;

	size_t bitSize;
	if (getBitSize(bitSize))
	{
		size = ((bitSize % 8) == 0) ? bitSize / 8 : bitSize / 8 + 1;
		state = true;
	}

	return state;
}

bool Variant::getBitSize(size_t &size) const
{
	bool state = false;

	if(_value.type() == typeid(boost::dynamic_bitset<>)) 
	{
		boost::dynamic_bitset<> bitset = boost::any_cast<boost::dynamic_bitset<> >(_value);
		size = bitset.size();
		state = true;
	}else if (_value.type() == typeid(char *) || _value.type() == typeid(const char *))
	{
		const char *data = boost::any_cast<const char *>(_value);
		size = strlen(data) * 8;
		state = true;
	}else if (_value.type() == typeid(uint8_t))
	{
		size = sizeof(uint8_t) * 8;
		state = true;
	}else if (_value.type() == typeid(int16_t)) 
	{
		size = sizeof(int16_t) * 8;
		state = true;
	}else if (_value.type() == typeid(uint16_t)) 
	{
		size = sizeof(uint16_t) * 8;
		state = true;
	}else if (_value.type() == typeid(int32_t))
	{
		size = sizeof(int32_t) * 8;
		state = true;
	}else if (_value.type() == typeid(uint32_t))
	{
		size = sizeof(uint32_t) * 8;
		state = true;
	}else if (_value.type() == typeid(int64_t))
	{
		size = sizeof(int64_t) * 8;
		state = true;
	}else if (_value.type() == typeid(uint64_t))
	{
		size = sizeof(uint64_t) * 8;
		state = true;
	}else if (_value.type() == typeid(float))
	{
		size = sizeof(float) * 8;
		state = true;
	}else if (_value.type() == typeid(double))
	{
		size = sizeof(double) * 8;
		state = true;
	}else
	{
		state = false;
	}

	return state;
}

bool Variant::isEmpty() const
{
	return _value.empty();
}

bool Variant::getValue(boost::dynamic_bitset<> &v) const
{
    bool state = false;
    switch (getType())
    {
    case DT_BIT:
        {
            v = boost::any_cast<boost::dynamic_bitset<>>(_value);
            state = true;
        }
        break;
	case DT_UCHAR:
		{
			boost::dynamic_bitset<> a(v.size(), boost::any_cast<uint8_t>(_value));
			v = a;
			state = true;
		}
		break;
	case DT_SHORT:
	{
		boost::dynamic_bitset<> a(v.size(), boost::any_cast<int16_t>(_value));
		v = a;
		state = true;
	}
	break;
	case DT_USHORT:
		{
			boost::dynamic_bitset<> a(v.size(), boost::any_cast<uint16_t>(_value));
			v = a;
			state = true;
		}
		break;
	case DT_LONG:
		{
			boost::dynamic_bitset<> a(v.size(), boost::any_cast<int32_t>(_value));
			v = a;
			state = true;
		}
		break;
	case DT_ULONG:
		{
			boost::dynamic_bitset<> a(v.size(), boost::any_cast<uint32_t>(_value));
			v = a;
			state = true;
		}
		break;
	case DT_LONGLONG:
		{
			boost::dynamic_bitset<> a(v.size(), boost::any_cast<int64_t>(_value));
			v = a;
			state = true;
		}
		break;
	case DT_ULONGLONG:
		{
			boost::dynamic_bitset<> a(v.size(), boost::any_cast<uint64_t>(_value));
			v = a;
			state = true;
		}
		break;
    default:
        break;
    }

    return state;    
}

bool Variant::getValue(std::vector<uint8_t> &v) const
{
    bool state = false;
    switch (getType())
    {
    case DT_BIT:
        {
			boost::dynamic_bitset<> p = boost::any_cast<boost::dynamic_bitset<> >(_value);
			boost::to_block_range(p, std::back_inserter(v));
            state = true;
        }
        break;
    default:
        break;
    }

    return state;    
}

bool Variant::getValue(char *v) const
{
    bool state = false;
    switch (getType())
    {
    case DT_CHAR:
        {
			const char *str = boost::any_cast<const char *>(_value);
			memcpy(v, str, strlen(str));
			state = true;
        }
        break;
    default:
        break;
    }

    return state;    
}

bool Variant::getValue(std::string &v) const
{
    bool state = false;
    switch (getType())
    {
    case DT_CHAR:
        {
			const char *str = boost::any_cast<const char *>(_value);
			v = str;
			state = true;
        }
        break;
    default:
        break;
    }

    return state;    
}


bool Variant::getValue(uint8_t &v) const
{
    bool state = false;
    switch (getType())
    {
	case DT_BIT:
	{
		v = (uint8_t)(boost::any_cast<boost::dynamic_bitset<>>(_value)).to_ulong();
		state = true;
	}
	break;
    case DT_UCHAR:
	{
		v = boost::any_cast<uint8_t>(_value);
		state = true;
	}
	break;
    case DT_SHORT:	
	{
		v = (uint8_t)boost::any_cast<int16_t>(_value);
		state = true;
	}
	break;
    case DT_USHORT:	
	{
		v = (uint8_t)boost::any_cast<uint16_t>(_value);
		state = true;
	}
	break;
    case DT_LONG:
	{
		v = (uint8_t)boost::any_cast<int32_t>(_value);
		state = true;
	}
	break;
    case DT_ULONG:		
	{
		v = (uint8_t)boost::any_cast<uint32_t>(_value);
		state = true;
	}
	break;
    case DT_LONGLONG:	
	{
		v = (uint8_t)boost::any_cast<int64_t>(_value);
		state = true;
	}
	break;
    case DT_ULONGLONG:		
	{
		v = (uint8_t)boost::any_cast<uint64_t>(_value);
		state = true;
	}
	break;
    case DT_FLOAT:
		{
			v = (uint8_t)boost::any_cast<float>(_value);
			state = true;
		}
		break;
    case DT_DOUBLE:	
        {
			v = (uint8_t)boost::any_cast<double>(_value);
			state = true;
        }
        break;
    default:
        break;
	}

	return state;
}

bool Variant::getValue(int16_t &v) const
{
    bool state = false;
	switch (getType())
	{
	case DT_BIT:
	{
		v = (int16_t)(boost::any_cast<boost::dynamic_bitset<> >(_value)).to_ulong();
		state = true;
	}
	break;
	case DT_UCHAR:
	{
		v = (int16_t)boost::any_cast<uint8_t>(_value);
		state = true;
	}
	break;
	case DT_SHORT:
	{
		v = (int16_t)boost::any_cast<int16_t>(_value);
		state = true;
	}
	break;
	case DT_USHORT:
	{
		v = (int16_t)boost::any_cast<uint16_t>(_value);
		state = true;
	}
	break;
	case DT_LONG:
	{
		v = (int16_t)boost::any_cast<int32_t>(_value);
		state = true;
	}
	break;
	case DT_ULONG:
	{
		v = (int16_t)boost::any_cast<uint32_t>(_value);
		state = true;
	}
	break;
	case DT_LONGLONG:
	{
		v = (int16_t)boost::any_cast<int64_t>(_value);
		state = true;
	}
	break;
	case DT_ULONGLONG:
	{
		v = (int16_t)boost::any_cast<uint64_t>(_value);
		state = true;
	}
	break;
	case DT_FLOAT:
	{
		v = (int16_t)boost::any_cast<float>(_value);
		state = true;
	}
	break;
	case DT_DOUBLE:
	{
		v = (int16_t)boost::any_cast<double>(_value);
		state = true;
	}
	break;
	default:
		break;
	}

	return state;
}

bool Variant::getValue(uint16_t &v) const
{
    bool state = false;
	switch (getType())
	{
	case DT_BIT:
	{
		v = (uint16_t)(boost::any_cast<boost::dynamic_bitset<>>(_value)).to_ulong();
		state = true;
	}
	break;
	case DT_UCHAR:
	{
		v = (uint16_t)boost::any_cast<uint8_t>(_value);
		state = true;
	}
	break;
	case DT_SHORT:
	{
		v = (uint16_t)boost::any_cast<int16_t>(_value);
		state = true;
	}
	break;
	case DT_USHORT:
	{
		v = (uint16_t)boost::any_cast<uint16_t>(_value);
		state = true;
	}
	break;
	case DT_LONG:
	{
		v = (uint16_t)boost::any_cast<int32_t>(_value);
		state = true;
	}
	break;
	case DT_ULONG:
	{
		v = (uint16_t)boost::any_cast<uint32_t>(_value);
		state = true;
	}
	break;
	case DT_LONGLONG:
	{
		v = (uint16_t)boost::any_cast<int64_t>(_value);
		state = true;
	}
	break;
	case DT_ULONGLONG:
	{
		v = (uint16_t)boost::any_cast<uint64_t>(_value);
		state = true;
	}
	break;
	case DT_FLOAT:
	{
		v = (uint16_t)boost::any_cast<float>(_value);
		state = true;
	}
	break;
	case DT_DOUBLE:
	{
		v = (uint16_t)boost::any_cast<double>(_value);
		state = true;
	}
	break;
	default:
		break;
	}

	return state;
}

bool Variant::getValue(int32_t &v) const
{
    bool state = false;
	switch (getType())
	{
	case DT_BIT:
	{
		v = (int32_t)(boost::any_cast<boost::dynamic_bitset<>>(_value)).to_ulong();
		state = true;
	}
	break;
	case DT_UCHAR:
	{
		v = (int32_t)boost::any_cast<uint8_t>(_value);
		state = true;
	}
	break;
	case DT_SHORT:
	{
		v = (int32_t)boost::any_cast<int16_t>(_value);
		state = true;
	}
	break;
	case DT_USHORT:
	{
		v = (int32_t)boost::any_cast<uint16_t>(_value);
		state = true;
	}
	break;
	case DT_LONG:
	{
		v = (int32_t)boost::any_cast<int32_t>(_value);
		state = true;
	}
	break;
	case DT_ULONG:
	{
		v = (int32_t)boost::any_cast<uint32_t>(_value);
		state = true;
	}
	break;
	case DT_LONGLONG:
	{
		v = (int32_t)boost::any_cast<int64_t>(_value);
		state = true;
	}
	break;
	case DT_ULONGLONG:
	{
		v = (int32_t)boost::any_cast<uint64_t>(_value);
		state = true;
	}
	break;
	case DT_FLOAT:
	{
		v = (int32_t)boost::any_cast<float>(_value);
		state = true;
	}
	break;
	case DT_DOUBLE:
	{
		v = (int32_t)boost::any_cast<double>(_value);
		state = true;
	}
	break;
	default:
		break;
	}

	return state;
}

bool Variant::getValue(uint32_t &v) const
{
    bool state = false;
	switch (getType())
	{
	case DT_BIT:
	{
		v = (uint32_t)(boost::any_cast<boost::dynamic_bitset<>>(_value)).to_ulong();
		state = true;
	}
	break;
	case DT_UCHAR:
	{
		v = (uint32_t)boost::any_cast<uint8_t>(_value);
		state = true;
	}
	break;
	case DT_SHORT:
	{
		v = (uint32_t)boost::any_cast<int16_t>(_value);
		state = true;
	}
	break;
	case DT_USHORT:
	{
		v = (uint32_t)boost::any_cast<uint16_t>(_value);
		state = true;
	}
	break;
	case DT_LONG:
	{
		v = (uint32_t)boost::any_cast<int32_t>(_value);
		state = true;
	}
	break;
	case DT_ULONG:
	{
		v = (uint32_t)boost::any_cast<uint32_t>(_value);
		state = true;
	}
	break;
	case DT_LONGLONG:
	{
		v = (uint32_t)boost::any_cast<int64_t>(_value);
		state = true;
	}
	break;
	case DT_ULONGLONG:
	{
		v = (uint32_t)boost::any_cast<uint64_t>(_value);
		state = true;
	}
	break;
	case DT_FLOAT:
	{
		v = (uint32_t)boost::any_cast<float>(_value);
		state = true;
	}
	break;
	case DT_DOUBLE:
	{
		v = (uint32_t)boost::any_cast<double>(_value);
		state = true;
	}
	break;
	default:
		break;
	}

	return state;
}


bool Variant::getValue(int64_t &v) const
{
    bool state = false;
	switch (getType())
	{
	case DT_BIT:
	{
		v = (int64_t)(boost::any_cast<boost::dynamic_bitset<>>(_value)).to_ulong();
		state = true;
	}
	break;
	case DT_UCHAR:
	{
		v = (int64_t)boost::any_cast<uint8_t>(_value);
		state = true;
	}
	break;
	case DT_SHORT:
	{
		v = (int64_t)boost::any_cast<int16_t>(_value);
		state = true;
	}
	break;
	case DT_USHORT:
	{
		v = (int64_t)boost::any_cast<uint16_t>(_value);
		state = true;
	}
	break;
	case DT_LONG:
	{
		v = (int64_t)boost::any_cast<int32_t>(_value);
		state = true;
	}
	break;
	case DT_ULONG:
	{
		v = (int64_t)boost::any_cast<uint32_t>(_value);
		state = true;
	}
	break;
	case DT_LONGLONG:
	{
		v = (int64_t)boost::any_cast<int64_t>(_value);
		state = true;
	}
	break;
	case DT_ULONGLONG:
	{
		v = (int64_t)boost::any_cast<uint64_t>(_value);
		state = true;
	}
	break;
	case DT_FLOAT:
	{
		v = (int64_t)boost::any_cast<float>(_value);
		state = true;
	}
	break;
	case DT_DOUBLE:
	{
		v = (int64_t)boost::any_cast<double>(_value);
		state = true;
	}
	break;
	default:
		break;
	}

	return state;
}

bool Variant::getValue(uint64_t &v) const
{
    bool state = false;
	switch (getType())
	{
	case DT_BIT:
	{
		v = (uint64_t)(boost::any_cast<boost::dynamic_bitset<>>(_value)).to_ulong();
		state = true;
	}
	break;
	case DT_UCHAR:
	{
		v = (uint64_t)boost::any_cast<uint8_t>(_value);
		state = true;
	}
	break;
	case DT_SHORT:
	{
		v = (uint64_t)boost::any_cast<int16_t>(_value);
		state = true;
	}
	break;
	case DT_USHORT:
	{
		v = (uint64_t)boost::any_cast<uint16_t>(_value);
		state = true;
	}
	break;
	case DT_LONG:
	{
		v = (uint64_t)boost::any_cast<int32_t>(_value);
		state = true;
	}
	break;
	case DT_ULONG:
	{
		v = (uint64_t)boost::any_cast<uint32_t>(_value);
		state = true;
	}
	break;
	case DT_LONGLONG:
	{
		v = (uint64_t)boost::any_cast<int64_t>(_value);
		state = true;
	}
	break;
	case DT_ULONGLONG:
	{
		v = (uint64_t)boost::any_cast<uint64_t>(_value);
		state = true;
	}
	break;
	case DT_FLOAT:
	{
		v = (uint64_t)boost::any_cast<float>(_value);
		state = true;
	}
	break;
	case DT_DOUBLE:
	{
		v = (uint64_t)boost::any_cast<double>(_value);
		state = true;
	}
	break;
	default:
		break;
	}

	return state;
}

bool Variant::getValue(float &v) const
{
    bool state = false;
	switch (getType())
	{
	case DT_BIT:
	{
		v = (float)(boost::any_cast<boost::dynamic_bitset<>>(_value)).to_ulong();
		state = true;
	}
	break;
	case DT_UCHAR:
	{
		v = (float)boost::any_cast<uint8_t>(_value);
		state = true;
	}
	break;
	case DT_SHORT:
	{
		v = (float)boost::any_cast<int16_t>(_value);
		state = true;
	}
	break;
	case DT_USHORT:
	{
		v = (float)boost::any_cast<uint16_t>(_value);
		state = true;
	}
	break;
	case DT_LONG:
	{
		v = (float)boost::any_cast<int32_t>(_value);
		state = true;
	}
	break;
	case DT_ULONG:
	{
		v = (float)boost::any_cast<uint32_t>(_value);
		state = true;
	}
	break;
	case DT_LONGLONG:
	{
		v = (float)boost::any_cast<int64_t>(_value);
		state = true;
	}
	break;
	case DT_ULONGLONG:
	{
		v = (float)boost::any_cast<uint64_t>(_value);
		state = true;
	}
	break;
	case DT_FLOAT:
	{
		v = (float)boost::any_cast<float>(_value);
		state = true;
	}
	break;
	case DT_DOUBLE:
	{
		v = (float)boost::any_cast<double>(_value);
		state = true;
	}
	break;
	default:
		break;
	}

	return state;
}

bool Variant::getValue(double &v) const
{
    bool state = false;
	switch (getType())
	{
	case DT_BIT:
	{
		v = (double)(boost::any_cast<boost::dynamic_bitset<>>(_value)).to_ulong();
		state = true;
	}
	break;
	case DT_UCHAR:
	{
		v = (double)boost::any_cast<uint8_t>(_value);
		state = true;
	}
	break;
	case DT_SHORT:
	{
		v = (double)boost::any_cast<int16_t>(_value);
		state = true;
	}
	break;
	case DT_USHORT:
	{
		v = (double)boost::any_cast<uint16_t>(_value);
		state = true;
	}
	break;
	case DT_LONG:
	{
		v = (double)boost::any_cast<int32_t>(_value);
		state = true;
	}
	break;
	case DT_ULONG:
	{
		v = (double)boost::any_cast<uint32_t>(_value);
		state = true;
	}
	break;
	case DT_LONGLONG:
	{
		v = (double)boost::any_cast<int64_t>(_value);
		state = true;
	}
	break;
	case DT_ULONGLONG:
	{
		v = (double)boost::any_cast<uint64_t>(_value);
		state = true;
	}
	break;
	case DT_FLOAT:
	{
		v = (double)boost::any_cast<float>(_value);
		state = true;
	}
	break;
	case DT_DOUBLE:
	{
		v = (double)boost::any_cast<double>(_value);
		state = true;
	}
	break;
	default:
		break;
	}

	return state;
}

}
