//std
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <utility>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <cstdlib>
#include <cassert>

#include "hsl.h"

using namespace std;

int main()
{
    try
    {
        std::string name = "d:\\test.hsp";
        hsl::Reader reader(name);
        if (!reader.open())
        {
            throw std::runtime_error(std::string("Can not open ") + name);
        }

        hsl::Header const& h = reader.getHeader();
        
        cout << "File name: " << name << '\n';
		uint32_t va, vi;
		va = h.getVersionMajor();
		vi = h.getVersionMinor();
        cout << "Version  : " << va << "." << vi << '\n';
        cout << "Signature: " << h.getFileSignature() << '\n';
        cout << "Points count: " << h.getPointRecordsCount() << '\n';
        cout << "Points by return: ";
		std::vector<size_t> records = h.getPointRecordsByReturnCount();
		if (records.size() > 0)
			std::copy(records.begin(), records.end(), ostream_iterator<uint32_t>(std::cout, " "));
        cout << std::endl;

        uint32_t count = 0;

		size_t bandCount = h.getSchema().getBandCount();
		hsl::Band band;

		// make sure band data type is 16-bit integer
		for (size_t count = 0; count < bandCount; count++)
		{
			if (h.getSchema().getBand(count, band))
			{
				if (band.getDataType() != hsl::DT_SHORT)
					return -1;
			}
			else
			{
				cout << "Error in reading band data type." << std::endl;
				return -1;
			}
		}

		std::vector<int16_t> values;
		values.resize(bandCount);

//		int16_t vv[3];

        while (reader.readNextPoint(true))
        {
            hsl::Point const& p = reader.getPoint();
			cout << "xyz: " << std::fixed << std::setprecision(6)
				<< p[0] << ", " << p[1] << ", " << p[2]
				<< std::endl;

			if (p.getBandValues(0, bandCount, (unsigned char *)values.data(), bandCount * sizeof(int16_t)))
//			if (p.getBandValues(0, bandCount, (unsigned char *)vv, bandCount * sizeof(int16_t)))
			{
				cout << "Band values: " << std::fixed << std::setprecision(6);
				for (size_t i = 0; i < bandCount - 1; i++)
				{
//					cout << vv[i] << ", ";
					cout << values[i] << ", ";
				}
//				cout << vv[bandCount - 1] << std::endl;
				cout << values[bandCount - 1] << std::endl;
			}

			hsl::Variant variant;
			hsl::VariantArray va;
			p.getValuesById(hsl::FI_NumberOfReturns, va);
			variant = va[0];
			uint8_t noOfReturns, classification;
			variant.getValue(noOfReturns);
			p.getValuesById(hsl::FI_Classification, va);
			variant = va[0];
			variant.getValue(classification);
			cout << "Number of returns: " << std::fixed << std::setprecision(0)
				<< (unsigned short)noOfReturns << ", " << "Classification: " << (unsigned short)classification
				<< std::endl;

			if (p.hasWaveformData())
			{
				cout << "Number of waveform bands: " << p.getWaveformBandCount() << std::endl;
				std::vector<uint8_t> data = p.getWaveformData();
				//TODO: process waveform data accordingly
			}

            count++;
        }

        if (reader.getHeader().getPointRecordsCount() != count)
            throw std::runtime_error("read incorrect number of point records");

		reader.close();
    }
    catch (std::exception const& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown error\n";
    }

    return 0;
}
