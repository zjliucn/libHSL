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

#include <stdexcept>
#include <string>

namespace hsl {

// Exception reporting invalid point data.
class invalid_point_data : public std::runtime_error
{
public:

    invalid_point_data(std::string const& msg, unsigned int who)
        : std::runtime_error(msg), _who(who)
    {}

    unsigned int who() const
    {
        return _who;
    }

private:

    unsigned int _who;
};

class libhsl_error : public std::runtime_error
{
public:

    libhsl_error(std::string const& msg)
        : std::runtime_error(msg)
    {}
};

class invalid_expression : public libhsl_error
{
public:

    invalid_expression(std::string const& msg)
        : libhsl_error(msg)
    {}
};

class invalid_format : public libhsl_error
{
public:

    invalid_format(std::string const& msg)
        : libhsl_error(msg)
    {}
};

class invalid_band_datatype : public libhsl_error
{
public:

	invalid_band_datatype(std::string const& msg)
		: libhsl_error(msg)
	{}
};

// use this for attempts to use a feature not compiled in
class configuration_error : public libhsl_error
{
public:
    configuration_error(std::string const& msg)
        : libhsl_error(msg)
    {}
};

// use this for code still under development
class not_yet_implemented : public libhsl_error
{
public:
    not_yet_implemented(std::string const& msg)
        : libhsl_error(msg)
    {}
};

}
