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


#include "WaveformPacketRecord.h"
#include <memory>

namespace hsl
{

  WaveformPacketRecord::WaveformPacketRecord() : _desc(std::vector<WaveformPacketDataDefinition>()), _data(std::vector<RawWaveformPacketData>())
  {
  }

  WaveformPacketRecord::WaveformPacketRecord(const WaveformPacketDataDefinition &def, const RawWaveformPacketData &data) : _desc(std::vector<WaveformPacketDataDefinition>()), _data(std::vector<RawWaveformPacketData>())
  {
    _desc.push_back(def);
    _data.push_back(data);
	updateByteOffset();
  }

  WaveformPacketRecord::~WaveformPacketRecord()
  {
  }

  void WaveformPacketRecord::addRawWaveformPacket(const WaveformPacketDataDefinition &def, const RawWaveformPacketData &data)
  {
	_desc.push_back(def);
	_data.push_back(data);

	//make sure the size is correct
	_desc.back().size = data.size();
	updateByteOffset();
  }

  void WaveformPacketRecord::addRawWaveformPacket(const WaveformPacketDataDefinition &def, const char *data, size_t size)
  {
    _desc.push_back(def);
	RawWaveformPacketData raw;
    raw.resize(size);
    memcpy(&raw[0], data, size);
	_data.push_back(raw);

	//make sure the offset and size is updated
	_desc.back().size = size;
	updateByteOffset();
  }

  bool WaveformPacketRecord::toWaveformData(std::vector<uint8_t> &data) const
  {
	const size_t bandCountBytes = sizeof(uint16_t);

	size_t byteCount = bandCountBytes;
    for (size_t i = 0; i < _data.size(); i++)
    {
		byteCount += _data[i].size();
    }

    const size_t c = sizeof(WaveformPacketDataDefinition);
    data.resize(_desc.size() * c + byteCount);

	// update band counts
	uint8_t* p_buf = static_cast<uint8_t *>(&data[0]);
	uint16_t* p_data = reinterpret_cast<uint16_t *>(p_buf);
	*p_data = _desc.size();

    for (size_t i = 0; i < _desc.size(); i++)
    {
		memcpy(&data[0] + bandCountBytes + i * c, &_desc[i], c);
    }

    size_t offset = c * _desc.size();
    for (size_t i = 0; i < _data.size(); i++)
    {
		memcpy(&data[0] + bandCountBytes + offset, _data[i].data(), _data[i].size());
		offset += _data[i].size();
    }

    return true;
  }

  void WaveformPacketRecord::updateByteOffset()
  {
	  const size_t bandCountBytes = sizeof(uint16_t);
	  const size_t c = sizeof(WaveformPacketDataDefinition);
	  size_t dataBytes = 0;
	  for (size_t i = 0; i < _data.size(); i++)
	  {
		  _desc[i].byteOffset = bandCountBytes + _desc.size() * c + dataBytes;
		  dataBytes += _data[i].size();
	  }
  }

} // namespace hsl