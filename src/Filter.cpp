/******************************************************************************
 * Copyright (c) 2010, Howard Butler
 *
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following 
 * conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided 
 *       with the distribution.
 *     * Neither the name of the Martin Isenburg or Iowa Department 
 *       of Natural Resources nor the names of its contributors may be 
 *       used to endorse or promote products derived from this software 
 *       without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 ****************************************************************************/

#include "Filter.h"
#include <boost/cstdint.hpp>
#include <vector>
#include "Variant.h"

using namespace boost;

namespace hsl { 

ClassificationFilter::ClassificationFilter( std::vector<hsl::Classification> classes )
    : FilterInterface(eInclusion)
    , m_classes(classes) 
{
}

bool ClassificationFilter::filter(const Point& p)
{
    VariantArray values;
    uint8_t classCode;
    if (p.getValuesById(FI_Classification, values))
    {
        classCode = boost::any_cast<uint8_t>(values[0].getValue());
    }else
    {
        return false;
    }    

    Classification c(classCode);
    
    // If the user gave us an empty set of classes to filter
    // we're going to return true regardless
    bool output = true;
    for (class_list_type::const_iterator it = m_classes.begin(); it != m_classes.end(); ++it) {
        if (c == *it) {
            if (GetType() == eInclusion) {
                output = true;
            } else {
                output = false;
            }
            break;
        } else {
            output = false;
        }
    }
    return output;
}

BoundsFilter::BoundsFilter( double minx, double miny, double maxx, double maxy ) : FilterInterface(eInclusion)
{
    bounds = Bounds<double>(minx, miny, maxx, maxy);
}

BoundsFilter::BoundsFilter( double minx, double miny, double minz, double maxx, double maxy, double maxz ) : FilterInterface(eInclusion)
{
    bounds = Bounds<double>(minx, miny, minz, maxx, maxy, maxz);
}

BoundsFilter::BoundsFilter( Bounds<double> const& b) : FilterInterface(eInclusion)
{
    bounds = b;
}
bool BoundsFilter::filter(const Point& p)
{
    return bounds.contains(p);
    // lasinfo --extent 630000.00 4834500.00 46.83 630300 4834600.00 150.00 TO_core_las_zoom.las
    
    // lasinfo --minx 630000.00 --miny 4834500.00 --minz 46.83 --maxx 630300 --maxy 4834600.00 --maxz 150.00 TO_core_las_zoom.las

}



ThinFilter::ThinFilter( uint32_t thin ) :
 hsl::FilterInterface(eInclusion)
{
    thin_amount = thin;
    thin_count = 0;
}



bool ThinFilter::filter(const hsl::Point& p)
{
    // FIXME: why p is not used? --mloskot
    // Because this filter is really just a counter.  
    // It throws out all points that aren't 
    // thin_count % thin_amount == 0. --hobu
    boost::ignore_unused_variable_warning(p);

    // If thin_amount == thin_count, we throw this one out.
    bool output = false;
    if (thin_amount == thin_count)
    {
        output = true;
        thin_count = 0;
    }
    
    thin_count = thin_count + 1;
    
    return output;
}


ReturnFilter::ReturnFilter( return_list_type returns, bool last_only )
    : FilterInterface(eInclusion)
    , m_returns(returns), last_only(last_only)
{
}

bool ReturnFilter::filter(const Point& p)
{

    if (last_only) {
        bool isLast = false;

        VariantArray values1, values2;
        uint8_t returnNum, numOfReturns;
        if (p.getValuesById(FI_NumberOfReturns, values1) && values1[0].getValue(numOfReturns) &&
            p.getValuesById(FI_ReturnNumber, values2) && values2[0].getValue(returnNum))
        {
            isLast = (returnNum == numOfReturns);
        }
        else
        {
            return false;
        }
        
        // If the type is switched to eExclusion, we'll throw out all last returns.
        if (GetType() == eExclusion)
            isLast = false;
        return isLast;
    }
    
    VariantArray values;
    uint16_t r;
    if (p.getValuesById(FI_ReturnNumber, values) && values[0].getValue(r))
        ;
    else
        return false;
    
    // If the user gave us an empty set of returns to filter
    // we're going to return true regardless
    bool output = true;
    for (return_list_type::const_iterator it = m_returns.begin(); it != m_returns.end(); ++it) {
        
        if (r == *it) {
            if (GetType() == eInclusion) {
                output = true;
            } else {
                output = false;
            }
            break;
        } else {
            output = false;
        }
    }
    return output;
}

ValidationFilter::ValidationFilter() :
 hsl::FilterInterface(eInclusion)
{

}



bool ValidationFilter::filter(const hsl::Point& p)
{

    bool output = false;
    if (p.isValid()){
        if (GetType() == eInclusion) {
            output = true;
        } else {
            output = false;
        }    
    }

    return output;
}


ColorFilter::ColorFilter(hsl::Color const& low, hsl::Color const& high) :
 hsl::FilterInterface(eInclusion), m_low(low), m_high(high)
{

}

ColorFilter::ColorFilter(hsl::Color::value_type low_red, 
                hsl::Color::value_type high_red,
                hsl::Color::value_type low_blue,
                hsl::Color::value_type high_blue,
                hsl::Color::value_type low_green,
                hsl::Color::value_type high_green) :
 hsl::FilterInterface(eInclusion), m_low(low_red, low_green, low_blue), m_high(high_red, high_blue, high_green)
{

}

bool ColorFilter::DoExclude()
{
    if (GetType() == eInclusion) {
        return true;
    } else {
        return false;
    }  
}


bool ColorFilter::filter(const hsl::Point& p)
{
    Color color;
    VariantArray values1, values2, values3;

    if (p.getValuesById(FI_Red, values1) && values1[0].getValue(color[0]) &&
        p.getValuesById(FI_Green, values2) && values2[0].getValue(color[1]) &&
        p.getValuesById(FI_Blue, values3) && values3[0].getValue(color[2]))
    {
        ;
    }else
    {
        return false;
    }    

    hsl::Color const& c = color;
    
    if (c.GetRed() < m_low.GetRed())
        return !DoExclude();
    
    if (c.GetRed() > m_high.GetRed())
        return !DoExclude();

    if (c.GetBlue() < m_low.GetBlue())
        return !DoExclude();
    
    if (c.GetBlue() > m_high.GetBlue())
        return !DoExclude();

    if (c.GetGreen() < m_low.GetGreen())
        return !DoExclude();
    
    if (c.GetGreen() > m_high.GetGreen())
        return !DoExclude();

    return DoExclude();
}

} // namespace liblas
