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

#include "hslLIB.h"
#include <stdio.h>
#include <string>
#include <memory>
#include <vector>
#include "hslDefinitions.h"
#include "Header.h"
#include "SpatialReference.h"


namespace hsl
{

class LIBHSL_API FileIO
{
public:
	FileIO();
	FileIO(std::string filename);
  	virtual ~FileIO();

	virtual bool open() = 0;
	virtual void close() = 0;	

	void setFilename(std::string filename);
	inline std::string getFilename() const { return _filename; };

    Header const& getHeader() const;
	Header& getHeader();
    void setHeader(Header const& header);

    /// Fetch the georeference
    SpatialReference getSRS() const;
    
    /// Set the georeference
    void setSRS(SpatialReference& srs);	

protected:
	bool loadHeader();
	bool writeHeader();
	bool updateHeader(Header const& header);	// update in-memory and disk header


protected:
	std::string 		_filename;
	FILE *				_fp;
    HeaderPtr			_header;
	SpatialReference	_srs;
};


}