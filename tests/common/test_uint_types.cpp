/***************************************************************************
 *  tests/common/test_uint_types.cpp
 *
 *  Part of the STXXL. See http://stxxl.org
 *
 *  Copyright (C) 2013 Timo Bingmann <tb@panthema.net>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#include <foxxll/common/uint_types.hpp>
#include <foxxll/verbose.hpp>

// forced instantiation
template class foxxll::uint_pair<uint8_t>;
template class foxxll::uint_pair<uint16_t>;

template <typename uint>
void dotest(unsigned int nbytes)
{
    // simple initialize
    uint a = 42;

    // check sizeof (again)
    STXXL_CHECK(sizeof(a) == nbytes);

    // count up 1024 and down again
    uint b = 0xFFFFFF00;
    uint b_save = b;

    uint64_t b64 = b;
    for (unsigned int i = 0; i < 1024; ++i)
    {
        STXXL_CHECK(b.u64() == b64);
        STXXL_CHECK(b.ull() == b64);
        STXXL_CHECK(b != a);
        ++b, ++b64;
    }

    STXXL_CHECK(b != b_save);

    for (unsigned int i = 0; i < 1024; ++i)
    {
        STXXL_CHECK(b.u64() == b64);
        STXXL_CHECK(b.ull() == b64);
        STXXL_CHECK(b != a);
        --b, --b64;
    }

    STXXL_CHECK(b.u64() == b64);
    STXXL_CHECK(b.ull() == b64);
    STXXL_CHECK(b == b_save);

    // check min and max value
    STXXL_CHECK(uint::min() <= a);
    STXXL_CHECK(uint::max() >= a);

    STXXL_CHECK(std::numeric_limits<uint>::min() < a);
    STXXL_CHECK(std::numeric_limits<uint>::max() > a);

    // check simple math
    a = 42;
    a = a + a;
    STXXL_CHECK(a == uint(84));
    STXXL_CHECK(a.ull() == 84);

    a += uint(0xFFFFFF00);
    STXXL_CHECK(a.ull() == 84 + (unsigned long long)(0xFFFFFF00));
}

int main()
{
    dotest<foxxll::uint40>(5);
    dotest<foxxll::uint48>(6);

    return 0;
}
