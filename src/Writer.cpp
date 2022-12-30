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


#include "Writer.h"
#include <vector>
#include <fstream>
#include <stdexcept>
#include <cstdlib> // std::size_t
#include <cassert>
#include <stddef.h>

#ifdef WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif


namespace hsl 
{


Writer::Writer() : FileIO(), _pointCount(0)
{
}

Writer::Writer(std::string filename, const Header &header) : FileIO(filename), _pointCount(0)
{
    setHeader(header);
	if (header.hasWaveformData())
		setPointCount(_header->getPointRecordsCount());
}

bool Writer::open()
{
	if (_filename == "")
		return false;

	_fp = fopen(_filename.c_str(), "wb");
	if (_fp == nullptr)
		return false;

	if (!writeHeader())
		return false;

	// extend file to guarantee we have enough space to store points 
	// so that waveform data can be written simultaneously.
	if (getHeader().hasWaveformData() && getHeader().isInternalWaveformData())
	{
		size_t size = _header->getDataOffset() + _totalPointCount * _header->getDataRecordLength();

#ifdef WIN32
		if (_chsize_s(fileno(_fp), size) != 0)
#else
		if (ftruncate(fileno(_fp), size) != 0)
#endif
		{
			throw hsl::libhsl_error("cannot allocate enough space to store point data.");
		}
	}

	return true;
}

void Writer::close()
{
    fclose(_fp);
}

void Writer::updatePointCount(uint64_t count)
{
    uint64_t out = _pointCount;

    if ( count != 0 ) 
    {
        out = count; 
    }

    _header->setPointRecordsCount(out);
    
    if (_fp == nullptr) 
        return;

    // Skip to first byte of number of point records data member
    size_t dataPos = offsetof(FileHeader, numberOfPointRecords);
    fseek(_fp, dataPos, SEEK_SET);
    fwrite(&out, sizeof(out), 1, _fp);
}

bool Writer::writePoint(Point & point)
{
	uint64_t offset = 0;
	uint32_t size = 0;
	// estimate and set waveform data address and size
	if (getHeader().hasWaveformData())
	{
		if (point.hasWaveformData())
		{
			std::vector<uint8_t> const& waveformData = point.getWaveformData();
			offset = _header->getDataOffset() + _totalPointCount * _header->getDataRecordLength();
			size = waveformData.size();
			point.setWaveformDataAddress(offset, size);
		}
	}

	std::vector<uint8_t> const& data = point.getData();
	fwrite(&data.front(), _header->getDataRecordLength(), 1, _fp);
	_pointCount++;

	if (getHeader().hasWaveformData())
	{
		if (point.hasWaveformData())
		{
			std::vector<uint8_t> const& waveformData = point.getWaveformData();
			if (getHeader().isInternalWaveformData())
			{
				// write waveform data to the file
				long pre = ftell(_fp);
				fseek(_fp, offset, SEEK_SET);
				fwrite(&waveformData.front(), size, 1, _fp);
				fseek(_fp, pre, SEEK_SET);    // back to previous position     
			}
			else
			{
				// write to an ancillary *.hsw waveform data file
				//TODO: save to hsw file
			}
		}
	}

	return true;
}

bool Writer::writePoint(const Point & point)
{
	Point pt = point;
	return writePoint(pt);
}

void Writer::setPointCount(size_t count)
{
    _totalPointCount = count;
}

Writer::~Writer()
{
    // Try to update the point count on our way out, but we don't really
    // care if we weren't able to write it.
    try
    {
        updatePointCount(_pointCount);
        
    } catch (std::runtime_error const&)
    {
        
    }
}

void Writer::setFilters(std::vector<FilterPtr> const& filters)
{
    _filters = filters;
}

std::vector<FilterPtr>  Writer::getFilters() const
{
    return _filters;
}

void Writer::setTransforms(std::vector<TransformPtr> const& transforms)
{
    _transforms = transforms;
}

std::vector<TransformPtr>  Writer::getTransforms() const
{
    return _transforms;
}

bool Writer::updateHeader(Header const& header)
{
	return FileIO::updateHeader(header);
}

}
