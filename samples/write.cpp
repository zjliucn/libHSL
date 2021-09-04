#include <algorithm>
#include <exception>
#include <stdexcept>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <cstdlib>
#include <cassert>
#include "hsl.h"

using namespace std;

int main()
{
    try
    {
        std::string name = "d:\\test.hsp";
        hsl::Header hdr;
        hdr.setVersionMajor(1);
        hdr.setVersionMinor(0);
		hdr.setDataFormat(hsl::PF_PointFormat5);
        hdr.setPointRecordsCount(1); // should be corrected automatically by writer

		hsl::Schema & schema = hdr.getSchema();
		// add band fields to header
		hsl::BandDesc band(hsl::DT_SHORT, "Band Value");
		schema.addBands(band, 3);

		// add waveform descriptor
		hsl::WaveformPacketDesc desc(0, 8, 0, 128, 10, 1, 0);
		hdr.addWaveformPacketDesc(desc);

        hsl::Writer writer(name, hdr);
        if (!writer.open())
        {
            throw std::runtime_error(std::string("Can not create ") + name);
        }

        hsl::Point p(&hdr);
        p.setCoordinates(10, 20, 30);
		
		hsl::WaveformPacketDataDefinition de;
		de.bandIndex = 3;
		de.descriptorIndex = 0;
		de.temporalOffset = 1234;
		de.dx = 0.1;
		de.dy = 0.1;
		de.dz = 0.1;
		hsl::RawWaveformPacketData raw;
		raw.assign(128, 23);

		hsl::WaveformPacketRecord record;
		record.addRawWaveformPacket(de, raw);
		p.setWaveformData(record);

		hsl::Variant variant;
		hsl::VariantArray va;
		variant.setValue(3);
		va.push_back(variant);
		p.setValuesById(hsl::FI_NumberOfReturns, va);
		va[0] = 10;
		p.setValuesById(hsl::FI_Classification, va);

		int16_t values[3] = { 1000, 5000, 123 };
		p.setBandValues(0, 3, (unsigned char *)values, sizeof(int16_t) * 3);

        writer.writePoint(p);
		writer.close();
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown error\n";
    }

    return 0;
}
