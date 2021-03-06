/***************************************************************************
 *  tests/mng/test_prefetch_pool.cpp
 *
 *  Part of the STXXL. See http://stxxl.org
 *
 *  Copyright (C) 2003 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 *  Copyright (C) 2009 Andreas Beckmann <beckmann@cs.uni-frankfurt.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

//! \example mng/test_prefetch_pool.cpp

#include <foxxll/mng.hpp>
#include <foxxll/mng/prefetch_pool.hpp>

#include <iostream>

#define BLOCK_SIZE (1024 * 512)

struct MyType
{
    int integer;
    char chars[5];
};

using block_type = foxxll::typed_block<BLOCK_SIZE, MyType>;

// forced instantiation
template class foxxll::prefetch_pool<block_type>;

int main()
{
    foxxll::prefetch_pool<block_type> pool(2);
    pool.resize(10);
    pool.resize(5);

    block_type* blk = new block_type;
    (*blk)[0].integer = 42;
    block_type::bid_type bids[2];
    foxxll::block_manager::get_instance()->new_blocks(foxxll::single_disk(), bids, bids + 2);
    blk->write(bids[0])->wait();
    blk->write(bids[1])->wait();

    pool.hint(bids[0]);
    pool.read(blk, bids[0])->wait();
    pool.read(blk, bids[1])->wait();

    delete blk;
}
