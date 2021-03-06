/***************************************************************************
 *  tests/mng/test_buf_streams.cpp
 *
 *  Part of the STXXL. See http://stxxl.org
 *
 *  Copyright (C) 2002 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

//! \example mng/test_buf_streams.cpp
//! This is an example of use of \c foxxll::buf_istream and \c foxxll::buf_ostream

#include <foxxll/mng.hpp>
#include <foxxll/mng/buf_istream.hpp>
#include <foxxll/mng/buf_istream_reverse.hpp>
#include <foxxll/mng/buf_ostream.hpp>

#include <iostream>

#define BLOCK_SIZE (1024 * 512)

using block_type = foxxll::typed_block<BLOCK_SIZE, unsigned>;
using bid_iterator_type = foxxll::BIDArray<BLOCK_SIZE>::iterator;

using buf_ostream_type = foxxll::buf_ostream<block_type, bid_iterator_type>;
using buf_istream_type = foxxll::buf_istream<block_type, bid_iterator_type>;
using buf_istream_reverse_type = foxxll::buf_istream_reverse<block_type, bid_iterator_type>;

// forced instantiations
template class foxxll::buf_ostream<block_type, foxxll::BIDArray<BLOCK_SIZE>::iterator>;
template class foxxll::buf_istream<block_type, foxxll::BIDArray<BLOCK_SIZE>::iterator>;
template class foxxll::buf_istream_reverse<block_type, foxxll::BIDArray<BLOCK_SIZE>::iterator>;

int main()
{
    const unsigned nblocks = 128;
    const unsigned nelements = nblocks * block_type::size;
    foxxll::BIDArray<BLOCK_SIZE> bids(nblocks);

    foxxll::block_manager* bm = foxxll::block_manager::get_instance();
    bm->new_blocks(foxxll::striping(), bids.begin(), bids.end());
    {
        buf_ostream_type out(bids.begin(), 2);
        for (unsigned i = 0; i < nelements; i++)
            out << i;
    }
    {
        buf_istream_type in(bids.begin(), bids.end(), 2);
        for (unsigned i = 0; i < nelements; i++)
        {
            unsigned prevalue = *in;
            unsigned value;
            in >> value;

            STXXL_CHECK2(value == i,
                         "Error at position " << std::hex << i << " (" << value << ") block " << (i / block_type::size));
            STXXL_CHECK(prevalue == value);
        }
    }
    {
        buf_istream_reverse_type in(bids.begin(), bids.end(), 2);
        for (unsigned i = 0; i < nelements; i++)
        {
            unsigned prevalue = *in;
            unsigned value;
            in >> value;

            STXXL_CHECK2(value == nelements - i - 1,
                         "Error at position " << std::hex << i << " (" << value << ") block " << (i / block_type::size));
            STXXL_CHECK(prevalue == value);
        }
    }
    bm->delete_blocks(bids.begin(), bids.end());

    return 0;
}
