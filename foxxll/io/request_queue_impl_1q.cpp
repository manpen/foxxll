/***************************************************************************
 *  foxxll/io/request_queue_impl_1q.cpp
 *
 *  Part of the STXXL. See http://stxxl.org
 *
 *  Copyright (C) 2002-2005 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 *  Copyright (C) 2009 Andreas Beckmann <beckmann@cs.uni-frankfurt.de>
 *  Copyright (C) 2009 Johannes Singler <singler@ira.uka.de>
 *  Copyright (C) 2013 Timo Bingmann <tb@panthema.net>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#include <foxxll/common/error_handling.hpp>
#include <foxxll/config.hpp>
#include <foxxll/io/request_queue_impl_1q.hpp>
#include <foxxll/io/serving_request.hpp>

#include <algorithm>

#if STXXL_MSVC >= 1700
 #include <windows.hpp>
#endif

#ifndef STXXL_CHECK_FOR_PENDING_REQUESTS_ON_SUBMISSION
#define STXXL_CHECK_FOR_PENDING_REQUESTS_ON_SUBMISSION 1
#endif

namespace foxxll {

struct file_offset_match
    : public std::binary_function<request_ptr, request_ptr, bool>
{
    bool operator () (
        const request_ptr& a,
        const request_ptr& b) const
    {
        // matching file and offset are enough to cause problems
        return (a->get_offset() == b->get_offset()) &&
               (a->get_file() == b->get_file());
    }
};

request_queue_impl_1q::request_queue_impl_1q(int n)
    : thread_state_(NOT_RUNNING), sem_(0)
{
    STXXL_UNUSED(n);
    start_thread(worker, static_cast<void*>(this), thread_, thread_state_);
}

void request_queue_impl_1q::add_request(request_ptr& req)
{
    if (req.empty())
        STXXL_THROW_INVALID_ARGUMENT("Empty request submitted to disk_queue.");
    if (thread_state_() != RUNNING)
        STXXL_THROW_INVALID_ARGUMENT("Request submitted to not running queue.");
    if (!dynamic_cast<serving_request*>(req.get()))
        STXXL_ERRMSG("Incompatible request submitted to running queue.");

#if STXXL_CHECK_FOR_PENDING_REQUESTS_ON_SUBMISSION
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (std::find_if(queue_.begin(), queue_.end(),
                         bind2nd(file_offset_match(), req))
            != queue_.end())
        {
            STXXL_ERRMSG("request submitted for a BID with a pending request");
        }
    }
#endif
    std::unique_lock<std::mutex> lock(queue_mutex_);
    queue_.push_back(req);

    sem_.signal();
}

bool request_queue_impl_1q::cancel_request(request_ptr& req)
{
    if (req.empty())
        STXXL_THROW_INVALID_ARGUMENT("Empty request canceled disk_queue.");
    if (thread_state_() != RUNNING)
        STXXL_THROW_INVALID_ARGUMENT("Request canceled to not running queue.");
    if (!dynamic_cast<serving_request*>(req.get()))
        STXXL_ERRMSG("Incompatible request submitted to running queue.");

    bool was_still_in_queue = false;
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_type::iterator pos
            = std::find(queue_.begin(), queue_.end(), req);

        if (pos != queue_.end())
        {
            queue_.erase(pos);
            was_still_in_queue = true;
            lock.unlock();
            sem_.wait();
        }
    }

    return was_still_in_queue;
}

request_queue_impl_1q::~request_queue_impl_1q()
{
    stop_thread(thread_, thread_state_, sem_);
}

void* request_queue_impl_1q::worker(void* arg)
{
    self* pthis = static_cast<self*>(arg);

    for ( ; ; )
    {
        pthis->sem_.wait();

        {
            std::unique_lock<std::mutex> lock(pthis->queue_mutex_);
            if (!pthis->queue_.empty())
            {
                request_ptr req = pthis->queue_.front();
                pthis->queue_.pop_front();

                lock.unlock();

                //assert(req->nref() > 1);
                dynamic_cast<serving_request*>(req.get())->serve();
            }
            else
            {
                lock.unlock();

                pthis->sem_.signal();
            }
        }

        // terminate if it has been requested and queues are empty
        if (pthis->thread_state_() == TERMINATING) {
            if (pthis->sem_.wait() == 0)
                break;
            else
                pthis->sem_.signal();
        }
    }

    pthis->thread_state_.set_to(TERMINATED);

#if STXXL_MSVC >= 1700
    // Workaround for deadlock bug in Visual C++ Runtime 2012 and 2013, see
    // request_queue_impl_worker.cpp. -tb
    ExitThread(nullptr);
#else
    return nullptr;
#endif
}

} // namespace foxxll
// vim: et:ts=4:sw=4
