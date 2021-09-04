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
#include "FileIO.h"
#include "Point.h"
#include "Filter.h"
#include "Transform.h"


namespace hsl
{

class LIBHSL_API Writer : public FileIO
{
public:
	Writer();
	Writer(std::string filename, const Header &header);
  	virtual ~Writer();

	virtual bool open();
	virtual void close();

    /// write point to file.
	bool writePoint(const Point & point);
	bool writePoint(Point & point);

    /// Sets filters that are used to determine whether or not to 
    /// keep a point that was read from the file.  Filters have *no* 
    /// effect for reading data at specific locations in the file.  
    /// They only affect reading ReadNextPoint-style operations
    /// Filters are applied *before* transforms.
    void setFilters(std::vector<hsl::FilterPtr> const& filters);
    
    /// Gets the list of filters to be applied to points as they are read
    std::vector<hsl::FilterPtr> getFilters() const;

    /// Sets transforms to apply to points.  Points are transformed in 
    /// place *in the order* of the transform list.
    /// Filters are applied *before* transforms.  
    void setTransforms(std::vector<hsl::TransformPtr> const& transforms);

    /// Gets the list of transforms to be applied to points as they are read
    std::vector<hsl::TransformPtr> getTransforms() const;

protected:
    bool filterPoint(hsl::Point const& p);
    void transformPoint(hsl::Point& p);


	void setPointCount(size_t count);

    void updatePointCount(uint64_t count);

private:
    bool            _needHeaderCheck;
    uint64_t        _size;
    uint64_t        _current;
          
    PointPtr        _point;
    uint64_t        _pointCount;
    uint64_t        _totalPointCount;

    std::vector<hsl::FilterPtr>     _filters;
    std::vector<hsl::TransformPtr>  _transforms;
    std::vector<uint8_t>::size_type _recordSize;
};

typedef std::shared_ptr<Writer> WriterPtr;

}