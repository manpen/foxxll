/***************************************************************************
 *  tests/io/test_cancel.cpp
 *
 *  Part of the STXXL. See http://stxxl.org
 *
 *  Copyright (C) 2009-2011 Johannes Singler <singler@kit.edu>
 *  Copyright (C) 2009 Andreas Beckmann <beckmann@cs.uni-frankfurt.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#include <cstring>
#include <foxxll/common/aligned_alloc.hpp>
#include <foxxll/io.hpp>

//! \example io/test_cancel.cpp
//! This tests the request cancelation mechanisms.

using foxxll::file;

struct print_completion
{
    void operator () (foxxll::request* ptr, bool success)
    {
        std::cout << "Request completed: " << ptr
                  << " success: " << success << std::endl;
    }
};

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " filetype tempfile" << std::endl;
        return -1;
    }

    const size_t size = 16 * 1024 * 1024, num_blocks = 16;
    char* buffer = (char*)foxxll::aligned_alloc<4096>(size);
    memset(buffer, 0, size);

    foxxll::file_ptr file = foxxll::create_file(
        argv[1], argv[2],
        foxxll::file::CREAT | foxxll::file::RDWR | foxxll::file::DIRECT);

    file->set_size(num_blocks * size);
    foxxll::request_ptr req[num_blocks];

    //without cancelation
    std::cout << "Posting " << num_blocks << " requests." << std::endl;
    foxxll::stats_data stats1(*foxxll::stats::get_instance());
    unsigned i = 0;
    for ( ; i < num_blocks; i++)
        req[i] = file->awrite(buffer, i * size, size, print_completion());
    wait_all(req, num_blocks);
    std::cout << foxxll::stats_data(*foxxll::stats::get_instance()) - stats1;

    //with cancelation
    std::cout << "Posting " << num_blocks << " requests." << std::endl;
    foxxll::stats_data stats2(*foxxll::stats::get_instance());
    for (unsigned i = 0; i < num_blocks; i++)
        req[i] = file->awrite(buffer, i * size, size, print_completion());
    //cancel first half
    std::cout << "Canceling first " << num_blocks / 2 << " requests." << std::endl;
    size_t num_canceled = cancel_all(req, req + num_blocks / 2);
    std::cout << "Successfully canceled " << num_canceled << " requests." << std::endl;
    //cancel every second in second half
    for (unsigned i = num_blocks / 2; i < num_blocks; i += 2)
    {
        std::cout << "Canceling request " << &(*(req[i])) << std::endl;
        if (req[i]->cancel())
            std::cout << "Request canceled: " << &(*(req[i])) << std::endl;
        else
            std::cout << "Request not canceled: " << &(*(req[i])) << std::endl;
    }
    wait_all(req, num_blocks);
    std::cout << foxxll::stats_data(*foxxll::stats::get_instance()) - stats2;

    foxxll::aligned_dealloc<4096>(buffer);

    file->close_remove();

    return 0;
}
