/***************************************************************************
 *  foxxll/common/log.cpp
 *
 *  Part of the STXXL. See http://stxxl.org
 *
 *  Copyright (C) 2004-2005 Roman Dementiev <dementiev@ira.uka.de>
 *  Copyright (C) 2008 Johannes Singler <singler@ira.uka.de>
 *  Copyright (C) 2009 Andreas Beckmann <beckmann@cs.uni-frankfurt.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#include <foxxll/common/log.hpp>

#include <cstdlib>

namespace foxxll {

logger::logger() : waitlog_stream_(nullptr)
{
    const char* log_filename = getenv("STXXLLOGFILE");
    log_stream_.open(log_filename == nullptr ? "stxxl.log" : log_filename);
    const char* errlog_filename = getenv("STXXLERRLOGFILE");
    errlog_stream_.open(errlog_filename == nullptr ? "stxxl.errlog" : errlog_filename);
#ifdef STXXL_WAIT_LOG_ENABLED
    const char* waitlog_filename = getenv("STXXLWAITLOGFILE");
    if (waitlog_filename) {
        waitlog_stream_ = new std::ofstream(waitlog_filename);
        *waitlog_stream_ << "# time\trd_incr\twr_incr\tw_read\tw_write" << std::endl;
    }
#endif
}

logger::~logger()
{
    delete waitlog_stream_;
}

} // namespace foxxll

/******************************************************************************/
