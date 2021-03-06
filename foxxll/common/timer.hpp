/***************************************************************************
 *  foxxll/common/timer.hpp
 *
 *  Part of the STXXL. See http://stxxl.org
 *
 *  Copyright (C) 2002, 2005 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 *  Copyright (C) 2007-2009 Andreas Beckmann <beckmann@cs.uni-frankfurt.de>
 *  Copyright (C) 2008 Johannes Singler <singler@ira.uka.de>
 *  Copyright (C) 2013-2014 Timo Bingmann <tb@panthema.net>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#ifndef STXXL_COMMON_TIMER_HEADER
#define STXXL_COMMON_TIMER_HEADER

#include <foxxll/common/utils.hpp>
#include <foxxll/config.hpp>
#include <foxxll/verbose.hpp>
#include <tlx/string/format_si_iec_units.hpp>

#include <chrono>
#include <limits>
#include <string>

namespace foxxll {

//! \addtogroup support
//! \{

//! Returns number of seconds since the epoch, high resolution.
static inline double timestamp()
{
    return static_cast<double>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count()) / 1e6;
}

/*!
 * Class timer is a simple stop watch timer. It uses the timestamp() function
 * to get the current time when start() is called. Then, after some processing,
 * the function stop() functions can be called, or seconds() and other
 * accessors can be called directly.
 */
class timer
{
    //! boolean whether the stopwatch timer is currently running
    bool running;

    //! total accumulated time in seconds.
    double accumulated;

    //! last start time of the stopwatch
    double last_clock;

    //! return current timestamp
    static inline double timestamp()
    {
        return foxxll::timestamp();
    }

public:
    //! boolean indicating that this class does real timing
    static const bool is_real = true;

    //! initialize and optionally immediately start the timer
    explicit timer(bool start_immediately = false)
        : running(false), accumulated(0), last_clock(0)
    {
        if (start_immediately) start();
    }

    //! start timer
    void start()
    {
        running = true;
        last_clock = timestamp();
    }

    //! stop timer
    void stop()
    {
        running = false;
        accumulated += timestamp() - last_clock;
    }

    //! return accumulated time
    void reset()
    {
        accumulated = 0.;
        last_clock = timestamp();
    }

    //! return currently accumulated time in milliseconds
    double mseconds() const
    {
        if (running)
            return (accumulated + timestamp() - last_clock) * 1000.;

        return (accumulated * 1000.);
    }

    //! return currently accumulated time in microseconds
    double useconds() const
    {
        if (running)
            return (accumulated + timestamp() - last_clock) * 1000000.;

        return (accumulated * 1000000.);
    }

    //! return currently accumulated time in seconds (as double)
    double seconds() const
    {
        if (running)
            return (accumulated + timestamp() - last_clock);

        return (accumulated);
    }

    //! accumulate elapsed time from another timer
    timer& operator += (const timer& tm)
    {
#if STXXL_PARALLEL
#pragma omp atomic
#endif
        accumulated += tm.seconds();
        return *this;
    }

    //! direct <<-operator for ostream. Can be used for printing with std::cout.
    friend std::ostream& operator << (std::ostream& os, const timer& t)
    {
        return os << t.seconds() << 's';
    }
};

/*!
 * Class fake_timer is a drop-in replacement for timer, which does
 * nothing. Using the fake class, timers can quickly be disabled in release
 * builds, but still be available for debugging session.
 *
 * \see timer
 */
class fake_timer
{
public:
    //! boolean indicating that this class does NOT do real timing
    static const bool is_real = false;

    //! initialize and optionally immediately start the timer
    explicit fake_timer(bool = false)
    { }

    //! start timer
    void start()
    { }

    //! stop timer
    void stop()
    { }

    //! return accumulated time
    void reset()
    { }

    //! return currently accumulated time in milliseconds
    double mseconds() const
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    //! return currently accumulated time in microseconds
    double useconds() const
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    //! return currently accumulated time in seconds (as double)
    double seconds() const
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    //! accumulate elapsed time from another timer
    fake_timer& operator += (const fake_timer&)
    {
        return *this;
    }

    //! direct <<-operator for ostream. Can be used for printing with std::cout.
    friend std::ostream& operator << (std::ostream& os, const fake_timer& t)
    {
        return os << t.seconds() << 's';
    }
};

/*!
 * Simple scoped timer, which takes a text message and prints the duration
 * until the scope is destroyed.
 */
class scoped_print_timer
{
protected:
    //! message
    std::string m_message;

    //! bytes processed
    uint64_t m_bytes;

    //! timer
    foxxll::timer m_timer;

public:
    //! save message and start timer
    explicit scoped_print_timer(const std::string& message, const uint64_t bytes = 0)
        : m_message(message),
          m_bytes(bytes),
          m_timer(true)
    {
        STXXL_MSG("Starting " << message);
    }

    //! on destruction: tell the time
    ~scoped_print_timer()
    {
        if (m_bytes == 0) {
            STXXL_MSG("Finished "
                      << m_message
                      << " after " << m_timer.seconds() << " seconds");
        }
        else {
            double bps = (double)m_bytes / m_timer.seconds();

            STXXL_MSG("Finished "
                      << m_message
                      << " after " << m_timer.seconds() << " seconds. "
                      << "Processed " << tlx::format_iec_units(m_bytes) << "B"
                      << " @ " << tlx::format_iec_units((uint64_t)bps) << "B/s");
        }
    }

    //! constant access to enclosed timer
    const foxxll::timer & timer() const
    {
        return m_timer;
    }
};

//! \}

} // namespace foxxll

#endif // !STXXL_COMMON_TIMER_HEADER
// vim: et:ts=4:sw=4
