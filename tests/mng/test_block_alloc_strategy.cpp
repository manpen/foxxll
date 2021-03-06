/***************************************************************************
 *  tests/mng/test_block_alloc_strategy.cpp
 *
 *  instantiate all block allocation strategies
 *
 *  Part of the STXXL. See http://stxxl.org
 *
 *  Copyright (C) 2008 Andreas Beckmann <beckmann@cs.uni-frankfurt.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#include <foxxll/common/error_handling.hpp>
#include <foxxll/mng.hpp>
#include <foxxll/mng/block_alloc_strategy_interleaved.hpp>

template <typename strategy>
void test_strategy()
{
    STXXL_MSG(STXXL_PRETTY_FUNCTION_NAME);
    strategy s0;
    strategy s2(1, 3);
    foxxll::offset_allocator<strategy> o(1, s0);
    using interleaved = typename foxxll::interleaved_alloc_traits<strategy>::strategy;
    interleaved itl(23, strategy(1, 3));
    for (int i = 0; i < 16; ++i)
        STXXL_MSG(s0(i) << " " << s2(i) << " " << o(i) << " " << itl(i));

    int firstdisk = 0;
    int ndisks = 4;
    int nruns = 10;
    int runsize = 15;
    std::cout << "firstdisk=" << firstdisk << "  ndisks=" << ndisks << "  nruns=" << nruns << "  runsize=" << runsize;
    interleaved itl2(nruns, strategy(firstdisk, firstdisk + ndisks));
    for (int i = 0; i < nruns * runsize; ++i) {
        if (i % nruns == 0)
            std::cout << std::endl;
        std::cout << itl2(i) << " ";
    }
    std::cout << std::endl;
}

int main()
{
    foxxll::config* cfg = foxxll::config::get_instance();

    // instantiate the allocation strategies
    STXXL_MSG("Number of disks: " << cfg->disks_number());
    for (unsigned i = 0; i < cfg->disks_number(); ++i)
        STXXL_MSG(cfg->disk_path(i) << ", " << cfg->disk_size(i) << ", " << cfg->disk_io_impl(i));
    test_strategy<foxxll::striping>();
    test_strategy<foxxll::fully_random>();
    test_strategy<foxxll::simple_random>();
    test_strategy<foxxll::random_cyclic>();
    STXXL_MSG("Regular disks: [" << cfg->regular_disk_range().first << "," << cfg->regular_disk_range().second << ")");
    if (cfg->regular_disk_range().first != cfg->regular_disk_range().second)
        test_strategy<foxxll::random_cyclic_disk>();
    STXXL_MSG("Flash devices: [" << cfg->flash_range().first << "," << cfg->flash_range().second << ")");
    if (cfg->flash_range().first != cfg->flash_range().second)
        test_strategy<foxxll::random_cyclic_flash>();
    test_strategy<foxxll::single_disk>();
}
