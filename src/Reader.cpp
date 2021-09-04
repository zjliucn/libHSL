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


#include "Reader.h"
#include <cfloat>
#include "Point.h"
#include "Transform.h"
#include "Filter.h"


namespace hsl
{

Reader::Reader(std::string filename) : FileIO(filename), _needHeaderCheck(false), _size(0), 
_point(PointPtr(new Point(&DefaultHeader::get()))), _current(0), _filters(0), _transforms(0), _recordSize(0)
{
}

Reader::~Reader()
{
}


bool Reader::open()
{
	if (_filename == "")
		return false;

	_fp = fopen(_filename.c_str(), "rb");
	if (_fp == nullptr)
		return false;

	if (!loadHeader())
		return false;

	_point->setHeader(_header.get());
	reset();

	return true;
}

void Reader::close()
{
    fclose(_fp);
}

void Reader::reset()
{
     fseek(_fp, 0, SEEK_SET);

    // Reset sizes and set internal cursor to the beginning of file.
    _current = 0;
    _size = _header->getPointRecordsCount();

    _recordSize = _header->getSchema().getByteSize();
}

Point const& Reader::getPoint() const
{
    return *_point;
}
    
bool Reader::readNextPoint(bool readWaveform)
{
    if (_current == 0)
    {
        fseek(_fp, _header->getDataOffset(), SEEK_SET);
    }

    if (_current >= _size ){
        return false;
    } 

    if (_needHeaderCheck) 
    {
        if (!(_point->getHeader() == _header.get()))
            _point->setHeader(_header.get());
    }
    
    try
    {
        size_t count = fread(&_point->getData()[0], _recordSize, 1, _fp);
		if (count != 1)
			return false;

        ++_current;
        
    } catch (std::runtime_error&)
    {
        // If the stream is no good anymore, we're done reading points
        return false;
    }

    // Filter the points and continue reading until we either find 
    // one to keep or throw an exception.

    bool bLastPoint = false;
    
    if (!_filters.empty())
    {
        if (!filterPoint(*_point))
        {

            try
            {
                fread(&_point->getData().front(), _recordSize, 1, _fp);
                ++_current;
        
            } catch (std::runtime_error&)
            {
                // If the stream is no good anymore, we're done reading points
                return false;
            }

            while (!filterPoint(*_point))
            {

                try
                {
                    fread(&_point->getData().front(), _recordSize, 1, _fp);
                    ++_current;
        
                } catch (std::runtime_error&)
                {
                    // If the stream is no good anymore, we're done reading points
                    return false;
                }
                if (_current == _size) 
                {
                    bLastPoint = true;
                    break;
                }
            }
        }
    }


    if (!_transforms.empty())
    {
        transformPoint(*_point);
    }

    if (bLastPoint)
        return false;

    if (readWaveform)
    {
        if(!readWaveformData())
            return false;
    }

    return true;
}

Point const& Reader::readPointAt(std::size_t n, bool readWaveform)
{
    if (_size == n) {
        throw std::out_of_range("file has no more points to read, end of file reached");
    } else if (_size < n) {
        std::ostringstream msg;
        msg << "ReadPointAt:: Inputted value: " << n << " is greater than the number of points: " << _size;
        throw std::runtime_error(msg.str());
    } 

    long pos = n * _header->getDataRecordLength() + _header->getDataOffset();    

    fseek(_fp, pos, SEEK_SET);

    if (_needHeaderCheck) 
    {
        if (!(_point->getHeader() == _header.get()))
            _point->setHeader(_header.get());
    }
    
    fread(&_point->getData().front(), _recordSize, 1, _fp);

    if (!_transforms.empty())
    {
        transformPoint(*_point);
    }

    if (readWaveform)
    {
        if(!readWaveformData())
            throw std::out_of_range("file has no more waveform data to read, end of file reached");
    }

    return *_point;
}

bool Reader::readWaveformData()
{  
    try
    {
		uint64_t pos = 0;
		_point->getWaveformDataByteOffset(pos);
		uint32_t size = 0;
		_point->getWaveformDataSize(size);
        if (_point->isValid() && size > 0)   
        {
            long pre = ftell(_fp);
            fseek(_fp, pos, SEEK_SET);
            _point->getWaveformData().resize(size);         
            fread(&_point->getWaveformData().front(), size, 1, _fp);
            fseek(_fp, pre, SEEK_SET);    // back to previous position 
            return true;
        }       
    } catch (std::runtime_error&)
    {
        // If the stream is no good anymore, we're done reading waveform data
        return false;
    }

    return true;
}

bool Reader::seek(size_t n)
{
    if (_size == n) {
        throw std::out_of_range("file has no more points to read, end of file reached");
        return false;
    } else if (_size < n) {
        std::ostringstream msg;
        msg << "Seek:: Inputted value: " << n << " is greater than the number of points: " << _size;
        throw std::runtime_error(msg.str());
        return false;
    } 

    long pos = n * _header->getDataRecordLength() + _header->getDataOffset();
    fseek(_fp, pos, SEEK_SET);    
    _current = n;

    return true;
}

void Reader::transformPoint(hsl::Point& p)
{    

    // Apply the transforms to each point
    std::vector<hsl::TransformPtr>::const_iterator ti;

    for (ti = _transforms.begin(); ti != _transforms.end(); ++ti)
    {
        hsl::TransformPtr transform = *ti;
        transform->transform(p);
    }            
}

bool Reader::filterPoint(hsl::Point const& p)
{    
    // If there's no filters on this reader, we keep 
    // the point no matter what.
    if (_filters.empty() ) {
        return true;
    }

    std::vector<hsl::FilterPtr>::const_iterator fi;
    for (fi = _filters.begin(); fi != _filters.end(); ++fi)
    {
        hsl::FilterPtr filter = *fi;
        if (!filter->filter(p))
        {
            return false;
        }
    }
    return true;
}

void Reader::setFilters(std::vector<hsl::FilterPtr> const& filters)
{
    _filters = filters;
}

std::vector<hsl::FilterPtr> Reader::getFilters() const
{
    return _filters;
}

void Reader::setTransforms(std::vector<hsl::TransformPtr> const& transforms)
{
    _transforms = transforms;
    
    // Transforms are allowed to change the point, including moving the 
    // point's HeaderPtr.  We need to check if we need to set that 
    // back on any subsequent reads.
    if (_transforms.size() > 0)
    {
        for (std::vector<hsl::TransformPtr>::const_iterator i = transforms.begin(); i != transforms.end(); i++)
        {
            if (i->get()->ModifiesHeader())
                _needHeaderCheck = true;
        }
    }
}

std::vector<hsl::TransformPtr>  Reader::getTransforms() const
{
    return _transforms;
}

}