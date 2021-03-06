/***************************************************************************
 *  foxxll/common/utils.hpp
 *
 *  Part of the STXXL. See http://stxxl.org
 *
 *  Copyright (C) 2002-2006 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 *  Copyright (C) 2007-2009 Andreas Beckmann <beckmann@cs.uni-frankfurt.de>
 *  Copyright (C) 2008 Johannes Singler <singler@ira.uka.de>
 *  Copyright (C) 2013 Timo Bingmann <tb@panthema.net>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#ifndef STXXL_COMMON_UTILS_HEADER
#define STXXL_COMMON_UTILS_HEADER

#include <foxxll/common/types.hpp>
#include <foxxll/config.hpp>
#include <foxxll/msvc_compatibility.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace foxxll {

////////////////////////////////////////////////////////////////////////////

#if defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 7)))
#  define STXXL_ATTRIBUTE_UNUSED __attribute__ ((unused))
#else
#  define STXXL_ATTRIBUTE_UNUSED
#endif

////////////////////////////////////////////////////////////////////////////

//! Format any ostream-able type into a string
template <typename Type>
std::string to_str(const Type& t)
{
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

////////////////////////////////////////////////////////////////////////////

inline int64_t atoi64(const char* s)
{
#if STXXL_MSVC
    return _atoi64(s);
#else
    return atoll(s);
#endif
}

////////////////////////////////////////////////////////////////////////////

inline uint64_t atouint64(const char* s)
{
#if STXXL_MSVC
    return _strtoui64(s, nullptr, 10);
#else
    return strtoull(s, nullptr, 10);
#endif
}

////////////////////////////////////////////////////////////////////////////

template <typename Integral, typename Integral2>
inline
typename std::remove_const<Integral>::type
div_ceil(Integral n, Integral2 d)
{
#if 0   // ambiguous overload for std::div(unsigned_anything, unsigned_anything)
    using div_type = __typeof__(std::div(n, d));
    div_type result = std::div(n, d);
    return result.quot + (result.rem != 0);
#else
    return n / d + ((n % d) != 0);
#endif
}

////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__
#define HAVE_BUILTIN_EXPECT
#endif

#ifdef HAVE_BUILTIN_EXPECT
 #define LIKELY(c)   __builtin_expect((c), 1)
#else
 #define LIKELY(c)   c
#endif

#ifdef HAVE_BUILTIN_EXPECT
 #define UNLIKELY(c)   __builtin_expect((c), 0)
#else
 #define UNLIKELY(c)   c
#endif

////////////////////////////////////////////////////////////////////////////

inline size_t longhash1(uint64_t key_)
{
    key_ += ~(key_ << 32);
    key_ ^= (key_ >> 22);
    key_ += ~(key_ << 13);
    key_ ^= (key_ >> 8);
    key_ += (key_ << 3);
    key_ ^= (key_ >> 15);
    key_ += ~(key_ << 27);
    key_ ^= (key_ >> 31);
    return (size_t)key_;
}

////////////////////////////////////////////////////////////////////////////

template <class Type>
inline void swap_1D_arrays(Type* a, Type* b, size_t size)
{
    for (size_t i = 0; i < size; ++i)
        std::swap(a[i], b[i]);
}

////////////////////////////////////////////////////////////////////////////

//! round n up to next larger multiple of 2^power. example: (48,4) = 64, (48,3) = 48.
template <typename Integral>
inline Integral round_up_to_power_of_two(Integral n, unsigned power)
{
    Integral pot = Integral(1) << power, // = 0..0 1 0^power
        mask = pot - 1;                  // = 0..0 0 1^power
    if (n & mask)                        // n not divisible by pot
        return (n & ~mask) + pot;
    else
        return n;
}

////////////////////////////////////////////////////////////////////////////

template <class Container>
inline typename Container::value_type pop(Container& c)
{
    typename Container::value_type r = c.top();
    c.pop();
    return r;
}

template <class Container>
inline typename Container::value_type pop_front(Container& c)
{
    typename Container::value_type r = c.front();
    c.pop_front();
    return r;
}

template <class Container>
inline typename Container::value_type pop_back(Container& c)
{
    typename Container::value_type r = c.back();
    c.pop_back();
    return r;
}

template <class Container>
inline typename Container::value_type pop_begin(Container& c)
{
    typename Container::value_type r = *c.begin();
    c.erase(c.begin());
    return r;
}

////////////////////////////////////////////////////////////////////////////

} // namespace foxxll

#endif // !STXXL_COMMON_UTILS_HEADER
// vim: et:ts=4:sw=4
