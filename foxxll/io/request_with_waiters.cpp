/***************************************************************************
 *  foxxll/io/request_with_waiters.cpp
 *
 *  Part of the STXXL. See http://stxxl.org
 *
 *  Copyright (C) 2002 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 *  Copyright (C) 2008 Andreas Beckmann <beckmann@cs.uni-frankfurt.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#include <foxxll/common/onoff_switch.hpp>
#include <foxxll/io/request_with_waiters.hpp>

#include <algorithm>
#include <functional>
#include <mutex>

namespace foxxll {

bool request_with_waiters::add_waiter(onoff_switch* sw)
{
    // this lock needs to be obtained before poll(), otherwise a race
    // condition might occur: the state might change and notify_waiters()
    // could be called between poll() and insert() resulting in waiter sw
    // never being notified
    std::unique_lock<std::mutex> lock(m_waiters_mutex);

    if (poll())                     // request already finished
    {
        return true;
    }

    m_waiters.insert(sw);

    return false;
}

void request_with_waiters::delete_waiter(onoff_switch* sw)
{
    std::unique_lock<std::mutex> lock(m_waiters_mutex);
    m_waiters.erase(sw);
}

void request_with_waiters::notify_waiters()
{
    std::unique_lock<std::mutex> lock(m_waiters_mutex);
    std::for_each(m_waiters.begin(), m_waiters.end(),
                  std::mem_fun(&onoff_switch::on));
}

size_t request_with_waiters::num_waiters()
{
    std::unique_lock<std::mutex> lock(m_waiters_mutex);
    return m_waiters.size();
}

} // namespace foxxll
// vim: et:ts=4:sw=4
