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

#pragma once

// std
#include <stdexcept> // std::out_of_range
#include <cstdlib> // std::size_t
#include <string>
#include "hslDefinitions.h"

namespace hsl
{

/// Spatial Reference System
class LIBHSL_API SpatialReference
{
public:
    enum WKTModeFlag
    {
        eHorizontalOnly = 1,
        eCompoundOK = 2
    };

    /// Default constructor.
    SpatialReference();

    /// Destructor.
    /// If libgeotiff is enabled, deallocates libtiff and libgeotiff objects used internally.
    ~SpatialReference();

    /// Copy constryctor.
    SpatialReference(SpatialReference const& other);

    /// Assignment operator.
    SpatialReference& operator=(SpatialReference const& rhs);

    bool operator==(const SpatialReference& other) const;
    bool operator!=(const SpatialReference& other) const { return !(*this == other); }

    /// Returns the OGC WKT describing Spatial Reference System.
    /// If GDAL is linked, it uses GDAL's operations and methods to determine 
    /// the WKT.  If GDAL is not linked, no WKT is returned.
    /// \param mode_flag May be eHorizontalOnly indicating the WKT will not 
    /// include vertical coordinate system info (the default), or 
    /// eCompoundOK indicating the the returned WKT may be a compound 
    /// coordinate system if there is vertical coordinate system info 
    /// available.
    std::string GetWKT(WKTModeFlag mode_flag = eHorizontalOnly) const;
    std::string GetWKT(WKTModeFlag mode_flag, bool pretty) const;
    
    /// Sets the SRS using GDAL's OGC WKT. If GDAL is not linked, this 
    /// operation has no effect.
    /// \param v - a string containing the WKT string.  
    void SetWKT(std::string const& v);

    /// Sets the vertical coordinate system using geotiff key values.
    /// This operation should normally be done after setting the horizontal
    /// portion of the coordinate system with something like SetWKT(), 
    /// SetProj4(), SetGTIF() or SetFromUserInput()
    /// \param verticalCSType - An EPSG vertical coordinate system code, 
    /// normally in the range 5600 to 5799, or -1 if one is not available.
    /// \param citation - a textual description of the vertical coordinate 
    /// system or an empty string if nothing is available.
    /// \param verticalDatum - the EPSG vertical datum code, often in the 
    /// range 5100 to 5299 - implied by verticalCSType if that is provided, or 
    /// -1 if no value is available.
    /// \param verticalUnits - the EPSG vertical units code, often 9001 for Metre.
    void SetVerticalCS(int32_t verticalCSType, 
                       std::string const& citation = std::string(0),
                       int32_t verticalDatum = -1,
                       int32_t verticalUnits = 9001);

    /// Sets the SRS using GDAL's SetFromUserInput function. If GDAL is not linked, this 
    /// operation has no effect.
    /// \param v - a string containing the definition (filename, proj4, wkt, etc).  
    void SetFromUserInput(std::string const& v);

private:
    std::string m_wkt;
};

}

