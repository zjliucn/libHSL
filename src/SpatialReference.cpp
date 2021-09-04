/******************************************************************************
 * Copyright (c) 2009, Howard Butler
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


// GDAL OSR
#ifdef HAVE_GDAL

// Supress inclusion of cpl_serv.h per #194, perhaps remove one day
// when libgeotiff 1.4.0+ is widely used
#define CPL_SERV_H_INCLUDED

#include <ogr_srs_api.h>
#include <cpl_port.h>
#include <geo_normalize.h>
#include <geovalues.h>
#include <ogr_spatialref.h>
#include <gdal.h>
#include <xtiffio.h>
#include <cpl_multiproc.h>
#endif

#include "SpatialReference.h"
// boost
#include <boost/concept_check.hpp>
#include <boost/cstdint.hpp>
// std
#include <stdexcept>
#include <string>
#include <vector>

#ifdef HAVE_GDAL
#  include "cpl_conv.h"
#endif

using namespace boost;

namespace hsl {

SpatialReference::SpatialReference()
{
}

SpatialReference::SpatialReference(SpatialReference const& other)
    : m_wkt(other.m_wkt)
{
}

SpatialReference& SpatialReference::operator=(SpatialReference const& rhs)
{
    if (&rhs != this)
    {
        m_wkt = rhs.m_wkt;
    }
    return *this;
}

bool SpatialReference::operator==(const SpatialReference& input) const
{
#ifdef HAVE_GDAL

    OGRSpatialReferenceH current = OSRNewSpatialReference(GetWKT(eCompoundOK, false).c_str());
    OGRSpatialReferenceH other = OSRNewSpatialReference(input.GetWKT(eCompoundOK, false).c_str());

    int output = OSRIsSame(current, other);

    OSRDestroySpatialReference( current );
    OSRDestroySpatialReference( other );

    return bool(output == 1);

#else
    boost::ignore_unused_variable_warning(input);
    throw std::runtime_error ("SpatialReference equality testing not available without GDAL+libgeotiff support");
#endif

}

SpatialReference::~SpatialReference()
{
}

std::string SpatialReference::GetWKT( WKTModeFlag mode_flag) const
{
    return GetWKT(mode_flag, false);
}

/// Fetch the SRS as WKT
std::string SpatialReference::GetWKT(WKTModeFlag mode_flag , bool pretty) const
{
#ifndef HAVE_GDAL
    boost::ignore_unused_variable_warning(mode_flag);
    boost::ignore_unused_variable_warning(pretty);

    // we don't have a way of making this pretty, or of stripping the compound wrapper.
    return m_wkt;
#else

    // If we already have Well Known Text then try return it, possibly
    // after some preprocessing.
    if( m_wkt != "" )
    {
        std::string result_wkt = m_wkt;

        if( (mode_flag == eHorizontalOnly
             && strstr(result_wkt.c_str(),"COMPD_CS") != NULL)
            || pretty )
        {
            OGRSpatialReference* poSRS = (OGRSpatialReference*) OSRNewSpatialReference(result_wkt.c_str());
            char *pszWKT = NULL;

            if( mode_flag == eHorizontalOnly )
                poSRS->StripVertical();

            if (pretty)
                poSRS->exportToPrettyWkt(&pszWKT, FALSE );
            else
                poSRS->exportToWkt( &pszWKT );

            OGRSpatialReference::DestroySpatialReference( poSRS );

            result_wkt = pszWKT;
            CPLFree( pszWKT );
        }

        return result_wkt;
    }

    return std::string();
#endif
}

void SpatialReference::SetFromUserInput(std::string const& v)
{
#ifdef HAVE_GDAL

    char* poWKT = 0;
    const char* input = v.c_str();

    // OGRSpatialReference* poSRS = (OGRSpatialReference*) OSRNewSpatialReference(NULL);
    OGRSpatialReference srs(NULL);
    if (OGRERR_NONE != srs.SetFromUserInput(input))
    {
        throw std::invalid_argument("could not import coordinate system into OSRSpatialReference SetFromUserInput");
    }

    srs.exportToWkt(&poWKT);

    std::string tmp(poWKT);
    CPLFree(poWKT);

    SetWKT(tmp);
#else
    boost::ignore_unused_variable_warning(v);
    throw std::runtime_error("GDAL is not available, SpatialReference could not be set from WKT");
#endif
}

void SpatialReference::SetWKT(std::string const& v)
{
    m_wkt = v;
}

void SpatialReference::SetVerticalCS(boost::int32_t verticalCSType,
                                     std::string const& citation,
                                     boost::int32_t verticalDatum,
                                     boost::int32_t verticalUnits)
{
    boost::ignore_unused_variable_warning(citation);
    boost::ignore_unused_variable_warning(verticalUnits);
    boost::ignore_unused_variable_warning(verticalDatum);
    boost::ignore_unused_variable_warning(verticalCSType);
}

}

