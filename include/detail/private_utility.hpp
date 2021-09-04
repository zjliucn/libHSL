 #pragma once

#include <vector>
#include <cmath>
#include <boost/concept_check.hpp>
#include "endian.hpp"
#include "binary.hpp"


namespace hsl 
{
namespace detail
{


// From http://stackoverflow.com/questions/485525/round-for-float-in-c
inline double sround(double r) {
    return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}


/// Compile-time calculation size of array defined statically.
template <typename T, std::size_t N>
inline std::size_t static_array_size(T (&t)[N])
{
    boost::ignore_unused_variable_warning(t);
    return (sizeof(t) / sizeof(t[0]));
}

/// Simple RAII wrapper.
/// It's dedicated to use with types associated with custom deleter,
/// opaque pointers and C API objects.
template <typename T>
class raii_wrapper
{
    typedef void(*deleter_type)(T* p);

public:

    raii_wrapper(T* p, deleter_type d)
        : p_(p), del_(d)
    {
        assert(0 != p_);
        assert(0 != del_);
    }

    raii_wrapper& operator=(raii_wrapper const& rhs)
    {
        if (&rhs != this)
        {
            p_ = rhs.p_;
            del_ = rhs.del_;
        }
        return *this;        
    }
    
    
    ~raii_wrapper()
    {
        do_delete(p_);
    }

    void reset(T* p)
    {
        do_delete(p_);
        p_= p;
    }

    T* get() const
    {
        return p_;
    }

    void swap(raii_wrapper& other)
    {
        std::swap(p_, other.p_);
    }

    
private:

    raii_wrapper(raii_wrapper const& other);
    // raii_wrapper& operator=(raii_wrapper const& rhs);

    void do_delete(T* p)
    {
        assert(del_);
        if (0 != p)
            del_(p);
    }

    T* p_;
    deleter_type del_;
};


template <typename T> 
bool compare_distance(const T& actual, const T& expected);

template <typename T>
struct Point
{
    Point()
        : x(T()), y(T()), z(T())
    {}
    
    Point(T const& x, T const& y, T const& z)
        : x(x), y(y), z(z)
    {}

    bool equal(Point<T> const& other) const
    {
        return (compare_distance(x, other.x)
                && compare_distance(y, other.y)
                && compare_distance(z, other.z));
    }

    T x;
    T y;
    T z;
};

template <typename T>
bool operator==(Point<T> const& lhs, Point<T> const& rhs)
{
    return lhs.equal(rhs);
}

template <typename T>
bool operator!=(Point<T> const& lhs, Point<T> const& rhs)
{
    return (!lhs.equal(rhs));
}



template <typename T> 
bool compare_distance(const T& actual, const T& expected) 
{ 
    const T epsilon = std::numeric_limits<T>::epsilon();  
    const T diff = actual - expected; 

    if ( !((diff <= epsilon) && (diff >= -epsilon )) ) 
    { 
        return false; 
    } 

    return true;
}

template<typename T>
inline char* as_buffer(T& data)
{
    return static_cast<char*>(static_cast<void*>(&data));
}

template<typename T>
inline char* as_buffer(T* data)
{
    return static_cast<char*>(static_cast<void*>(data));
}

template<typename T>
inline char const* as_bytes(T const& data)
{
    return static_cast<char const*>(static_cast<void const*>(&data));
}

template<typename T>
inline char const* as_bytes(T const* data)
{
    return static_cast<char const*>(static_cast<void const*>(data));
}

// adapted from http://www.cplusplus.com/forum/beginner/3076/
template <typename IntegerType>
inline IntegerType bitsToInt(IntegerType& output,
                             std::vector<uint8_t> const& data, 
                             std::size_t index)
{
    binary::endian_value<IntegerType> value;
    value.template load<binary::little_endian_tag>(&data[0] + index);
    output = value;
    return output;
}

template <typename IntegerType>
inline void intToBits(IntegerType input, 
                      std::vector<uint8_t>& data, 
                      std::size_t index)
{
    binary::endian_value<IntegerType> value(input);
    value.template store<binary::little_endian_tag>(&data[0] + index);
}

}
}