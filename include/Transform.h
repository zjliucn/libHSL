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

#pragma once

#include <memory>
#include <vector>
#include <string>

#include "hslLIB.h"
#include "Point.h"
#include "Exception.h"
#include "SpatialReference.h"

namespace hsl {

/// Defines public interface to LAS transform implementation.
class LIBHSL_API TransformInterface
{
public:
    
    virtual bool transform(Point& point) = 0;
    virtual bool ModifiesHeader() = 0;
    virtual ~TransformInterface() {}
};

typedef std::shared_ptr<TransformInterface> TransformPtr;

class LIBHSL_API ReprojectionTransform: public TransformInterface
{
public:
    
    ReprojectionTransform(const SpatialReference& inSRS, const SpatialReference& outSRS);    
    ReprojectionTransform(const SpatialReference& inSRS, const SpatialReference& outSRS, Header const* new_header);    
    ~ReprojectionTransform();

    bool transform(Point& point);
    void SetHeader(Header* header) {m_new_header = header;}
    bool ModifiesHeader() { return true; }

private:

    Header const* m_new_header;
    
    typedef std::shared_ptr<void> ReferencePtr;
    typedef std::shared_ptr<void> TransformPtr;
    ReferencePtr m_in_ref_ptr;
    ReferencePtr m_out_ref_ptr;
    TransformPtr m_transform_ptr;

    ReprojectionTransform(ReprojectionTransform const& other);
    ReprojectionTransform& operator=(ReprojectionTransform const& rhs);
    
    void Initialize(SpatialReference const& inSRS, SpatialReference const& outSRS);
};

class LIBHSL_API TranslationTransform: public TransformInterface
{
public:
    
    TranslationTransform(std::string const& expression);
    ~TranslationTransform();

    bool transform(Point& point);
    bool ModifiesHeader() { return false; }
    
    enum OPER_TYPE
    {
        eOPER_MULTIPLY = 0, 
        eOPER_DIVIDE = 1, 
        eOPER_SUBTRACT = 2,  
        eOPER_ADD = 3,
        eOPER_NONE = -99
    };

    // Yes, Mateusz, I'm embarassed by this :)
    struct operation{
        OPER_TYPE oper;
        std::string dimension;
        double value;
        std::string expression;
        
        operation(std::string name) : oper(eOPER_NONE), dimension(name), value(0.0)
        {
        }
    };

private:

    TranslationTransform(TranslationTransform const& other);
    TranslationTransform& operator=(TranslationTransform const& rhs);
    
    operation GetOperation(std::string const& expression);
    
    std::vector<operation> operations;
    
    std::string m_expression;
};


class LIBHSL_API ColorFetchingTransform: public TransformInterface
{
public:
    
    ColorFetchingTransform( std::string const& datasource, 
                            std::vector<uint32_t> bands
                            );
    ColorFetchingTransform( std::string const& datasource, 
                            std::vector<uint32_t> bands,
                            Header const* header);
    
    void SetScaleFactor(uint32_t v) {m_scale = v; }
    ~ColorFetchingTransform();

    bool transform(Point& point);
    bool ModifiesHeader() { return true; }


private:

    Header const* m_new_header;
    
    typedef std::shared_ptr<void> DataSourcePtr;
    
    DataSourcePtr m_ds;
    std::string m_datasource;
    std::vector<uint32_t> m_bands;
    boost::array<double, 6> m_forward_transform;
	boost::array<double, 6> m_inverse_transform;
    uint32_t m_scale;

    ColorFetchingTransform(ColorFetchingTransform const& other);
    ColorFetchingTransform& operator=(ColorFetchingTransform const& rhs);
    
    void Initialize();
};

typedef std::shared_ptr<TransformInterface> TransformPtr;

}
