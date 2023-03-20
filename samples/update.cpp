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
        hsl::Updater updater(name);
        if (!updater.open())
        {
            throw std::runtime_error(std::string("Can not open ") + name);
        }

        hsl::Header const& h = updater.getHeader();
        
        cout << "File name: " << name << '\n';
		uint32_t vm, vi;
		vm = h.getVersionMajor();
		vi = h.getVersionMinor();
        cout << "Version  : " << vm << "." << vi << '\n';
        cout << "Signature: " << h.getFileSignature() << '\n';
        cout << "Points count: " << h.getPointRecordsCount() << '\n';
        cout << "Points by return: ";
		std::vector<size_t> records = h.getPointRecordsByReturnCount();
		if (records.size() > 0)
			std::copy(records.begin(), records.end(), ostream_iterator<uint32_t>(std::cout, " "));
        cout << std::endl;

        typedef std::pair<double, double> minmax_t;
        uint32_t i = 0;
        hsl::Variant variant;
        hsl::VariantArray va;
        uint8_t noOfReturns, classification;

        while (updater.readNextPoint(true))
        {
            hsl::Point const& p = updater.getPoint();
			cout << "xyz: " << std::fixed << std::setprecision(6)
				<< p[0] << ", " << p[1] << ", " << p[2]
				<< std::endl;
			double values[3] = { 0.0, 0.0, 0.0 };
			p.getBandValues(0, 3, (unsigned char *)values, sizeof(double) * 3);
			cout << "Band values: " << std::fixed << std::setprecision(6)
				<< values[0] << ", " << values[1] << ", " << values[2]
				<< std::endl;


			p.getValuesById(hsl::FI_NumberOfReturns, va);
			variant = va[0];
			variant.getValue(noOfReturns);
			p.getValuesById(hsl::FI_Classification, va);
			variant = va[0];
			variant.getValue(classification);
			cout << "Number of returns: " << std::fixed << std::setprecision(0)
				<< (unsigned short)noOfReturns << ", " << "Classification: " << classification
				<< std::endl;

			if (p.hasWaveformData())
			{
				cout << "Number of waveform bands: " << p.getWaveformBandCount() << std::endl;
				std::vector<uint8_t> data = p.getWaveformData();
			}

            i++;
        }

        classification = 2;
        va[0] = classification;

        updater.seek(0);
        updater.writeFieldValuesById(hsl::FI_Classification, va);

        if (updater.getHeader().getPointRecordsCount() != i)
            throw std::runtime_error("read incorrect number of point records");

        updater.close();
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
