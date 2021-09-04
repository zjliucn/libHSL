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


#pragma once

// boost
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/thread/once.hpp>
#include <boost/scoped_ptr.hpp>

// std
#include <iosfwd> // std::ostream
#include <string>
#include <memory>
#include <cstdlib> // std::size_t

namespace hsl {


// From the boost cookbook
// http://www.boostcookbook.com/Recipe:/1235044
// Warning: If T's constructor throws, instance() will return a null reference.

template<class T>

class Singleton : private boost::noncopyable
{

public:
    static T& get()
    {
        boost::call_once(init, flag);
        return *t;
    }

    static void init() // never throws
    {
        t.reset(new T());
    }

protected:
    ~Singleton() {}
     Singleton() {}

private:
     static boost::scoped_ptr<T> t;
     static boost::once_flag flag;

};


template<class T> boost::scoped_ptr<T> Singleton<T>::t(0);
template<class T> boost::once_flag Singleton<T>::flag = BOOST_ONCE_INIT;


}