/***************************************************************************
 *  foxxll/io/file.cpp
 *
 *  Part of the STXXL. See http://stxxl.org
 *
 *  Copyright (C) 2013 Timo Bingmann <tb@panthema.net>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#include "ufs_platform.hpp"
#include <foxxll/io/file.hpp>

namespace foxxll {

int file::unlink(const char* path)
{
    return ::unlink(path);
}

} // namespace foxxll

/******************************************************************************/
