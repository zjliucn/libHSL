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

        typedef std::pair<double, double> minmax_t;
        uint32_t i = 0;
        size_t bandCount = h.getSchema().getBandCount();
        hsl::Band band;
		bool hasBandData = false;
		hsl::DataType dataType = hsl::DT_UCHAR;
		if (bandCount != 0 && h.getSchema().getBand(0, band))
		{
			hasBandData = true;
			dataType = band.getDataType();
		}
        while (reader.readNextPoint(true))
        {
            hsl::Point const& p = reader.getPoint();
			cout << "xyz: " << std::fixed << std::setprecision(6)
				<< p[0] << ", " << p[1] << ", " << p[2]
				<< std::endl;
			double values[1] = { 0.0 };
			p.getBandValues(0, 1, (unsigned char *)values, sizeof(double) * 1);

			if (hasBandData)
			{
				switch (dataType)
				{
				case hsl::DT_UCHAR:
					cout << "Band values: " << std::fixed << std::setprecision(6)
						<< *(unsigned char*)values << std::endl;
					break;
				case hsl::DT_SHORT:
					cout << "Band values: " << std::fixed << std::setprecision(6)
						<< *(short*)values << std::endl;
					break;
				case hsl::DT_USHORT:
					cout << "Band values: " << std::fixed << std::setprecision(6)
						<< *(unsigned short*)values << std::endl;
					break;
				case hsl::DT_LONG:
					cout << "Band values: " << std::fixed << std::setprecision(6)
						<< *(int*)values << std::endl;
					break;
				case hsl::DT_ULONG:
					cout << "Band values: " << std::fixed << std::setprecision(6)
						<< *(unsigned int*)values << std::endl;
					break;
				case hsl::DT_LONGLONG:
					cout << "Band values: " << std::fixed << std::setprecision(6)
						<< *(int64_t*)values << std::endl;
					break;
				case hsl::DT_ULONGLONG:
					cout << "Band values: " << std::fixed << std::setprecision(6)
						<< *(uint64_t*)values << std::endl;
					break;
				case hsl::DT_FLOAT:
					cout << "Band values: " << std::fixed << std::setprecision(6)
						<< *(float*)values << std::endl;
					break;
				case hsl::DT_DOUBLE:
					cout << "Band values: " << std::fixed << std::setprecision(6)
						<< *(double*)values << std::endl;
					break;
				default:
					break;
				}
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
			}

            i++;
        }

        if (reader.getHeader().getPointRecordsCount() != i)
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
