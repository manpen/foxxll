#ifndef IOSTATS_HEADER
#define IOSTATS_HEADER

/***************************************************************************
 *            iostats.h
 *
 *  Sat Aug 24 23:54:50 2002
 *  Copyright  2002  Roman Dementiev
 *  dementiev@mpi-sb.mpg.de
 ****************************************************************************/

#include "../common/mutex.h"
#include "../common/utils.h"

#include <iostream>

__STXXL_BEGIN_NAMESPACE

  //! \addtogroup iolayer
  //!
  //! \{

#ifdef COUNT_WAIT_TIME
extern double stxxl::wait_time_counter;
#endif

	class disk_queue;

	//! \brief Collects varoius I/O statistics
	//! \remarks is a singleton
	class stats
	{
		friend class disk_queue;
		unsigned reads, writes;	// number of operations
		double t_reads, t_writes;	//  seconds spent in operations
		double p_reads, p_writes;	// seconds spent in parallel operations
		double p_begin_read, p_begin_write;	// start time of parallel operation
		double p_ios;	// seconds spent in all parallel I/O operations (read and write)
		double p_begin_io;
		int acc_ios;
		int acc_reads, acc_writes;	// number of requests, participating in parallel operation
		mutex read_mutex, write_mutex, io_mutex;
		static stats * instance;
		  stats ():reads (0),
			writes (0),
			t_reads (0.0),
			t_writes (0.0),
			p_reads (0.0),
			p_writes (0.0),
			p_begin_read (0.0),
			p_begin_write (0.0),
			p_ios (0.0),
			p_begin_io (0),
			acc_ios (0), acc_reads (0), acc_writes (0)
		{
		};
	public:
		//! \brief Call this function in order to access an instance of stats
		//! \return pointer to an instance of stats
		static stats *get_instance ()
		{
			if (!instance)
				instance = new stats ();
			return instance;
		};
		//! \brief Returns total number of reads
		//! \return total number of reads
		unsigned get_reads ()
		{
			return reads;
		};
		//! \brief Returns total number of writes
		//! \return total number of writes
		unsigned get_writes ()
		{
			return writes;
		};
		//! \brief Returns time spent in serving all read requests
		//! \remarks If there are \c n read requests that are served simultaneously
		//! \remarks in 1 second then the counter corresponding to the function increments by \c n seconds
		//! \return seconds spent in reading
		double get_read_time ()
		{
			return t_reads;
		};
		//! \brief Returns total time spent in serving all write requests
		//! \remarks If there are \c n write requests that are served simultaneously
		//! \remarks in 1 second then the counter corresponding to the function increments by \c n seconds
		//! \return seconds spent in writing
		double get_write_time ()
		{
			return t_writes;
		};
		//! \brief Returns time spent in reading (parallel read time)
		//! \remarks If there are \c n read requests that are served simultaneously
		//! \remarks in 1 second then the counter corresponding to the function increments by 1 second
		//! \return seconds spent in reading
		double get_pread_time()
		{
			return p_reads;
		};
		//! \brief Returns time spent in writing (parallel write time)
		//! \remarks If there are \c n write requests that are served simultaneously
		//! \remarks in 1 second then the counter corresponding to the function increments by 1 second
		//! \return seconds spent in writing
		double get_pwrite_time()
		{
			return p_writes;
		};
		//! \brief Returns time spent in I/O (parallel I/O time)
		//! \remarks If there are \c n \b any requests that are served simultaneously
		//! \remarks in 1 second then the counter corresponding to the function increments by 1 second
		//! \return seconds spent in I/O
		double get_pio_time()
		{
			return p_ios;
		};
		//! \brief Resets I/O time counters
		void reset()
		{
			read_mutex.lock ();
			//      assert(acc_reads == 0);
			if (acc_reads)
				std::cerr << "Warning: " << acc_reads <<
					" read(s) not yet finished" << std::
					endl;

			reads = 0;
			t_reads = 0;
			p_reads = 0.0;
			read_mutex.unlock ();
			write_mutex.lock ();
			//      assert(acc_writes == 0);
			if (acc_writes)
				std::cerr << "Warning: " << acc_writes <<
					" write(s) not yet finished" << std::
					endl;

			writes = 0;
			t_writes = 0.0;
			p_writes = 0.0;
			write_mutex.unlock ();

			io_mutex.lock ();
			//      assert(acc_ios == 0);
			if (acc_ios)
				std::cerr << "Warning: " << acc_ios <<
					" io(s) not yet finished" << std::
					endl;

			p_ios = 0.0;
			io_mutex.unlock ();

		};
		
		
#ifdef COUNT_WAIT_TIME
		//! \brief Resets I/O wait time counter
		void _reset_io_wait_time() { stxxl::wait_time_counter = 0.0; }
		//! \brief Returns I/O wait time counter
		//! \return number of seconds spent in I/O waiting functions 
		//!  \link request::wait request::wait \endlink,
		//!  \c wait_any and
		//!  \c wait_all 
		double get_io_wait_time() { return (stxxl::wait_time_counter); }
		//! \brief Increments I/O wait time counter
		//! \param val increment value in seconds
		//! \return new value of I/O wait time counter in seconds
		double increment_io_wait_time(double val) { return stxxl::wait_time_counter+= val; }
#else
		//! \brief Resets I/O wait time counter
		void _reset_io_wait_time() {}
		//! \brief Returns I/O wait time counter
		//! \return number of seconds spent in I/O waiting functions 
		//!  \link request::wait request::wait \endlink,
		//!  \c wait_any and
		//!  \c wait_all 
		//! \return number of seconds
		double get_io_wait_time() { return -1.0; }
		//! \brief Increments I/O wait time counter
		//! \param val increment value in seconds
		//! \return new value of I/O wait time counter in seconds
		double increment_io_wait_time(double val) { return -1.0; }
#endif
		
	protected:
		void write_started ()
		{
			write_mutex.lock ();
			double now = stxxl_timestamp ();
			writes++;
			double diff = now - p_begin_write;
			t_writes += double (acc_writes) * diff;
			p_begin_write = now;
			p_writes += (acc_writes++) ? diff : 0.0;
			write_mutex.unlock ();

			io_mutex.lock ();
			diff = now - p_begin_io;
			p_ios += (acc_ios++) ? diff : 0.0;
			p_begin_io = now;
			io_mutex.unlock ();
		}
		void write_finished ()
		{
			write_mutex.lock ();
			double now = stxxl_timestamp ();
			double diff = now - p_begin_write;
			t_writes += double (acc_writes) * diff;
			p_begin_write = now;
			p_writes += (acc_writes--) ? diff : 0.0;
			write_mutex.unlock ();

			io_mutex.lock ();
			diff = now - p_begin_io;
			p_ios += (acc_ios--) ? diff : 0.0;
			p_begin_io = now;
			io_mutex.unlock ();
		}
		void read_started ()
		{
			read_mutex.lock ();
			double now = stxxl_timestamp ();
			reads++;
			double diff = now - p_begin_read;
			t_reads += double (acc_reads) * diff;
			p_begin_read = now;
			p_reads += (acc_reads++) ? diff : 0.0;
			read_mutex.unlock ();

			io_mutex.lock ();
			diff = now - p_begin_io;
			p_ios += (acc_ios++) ? diff : 0.0;
			p_begin_io = now;
			io_mutex.unlock ();
		}
		void read_finished ()
		{
			read_mutex.lock ();
			double now = stxxl_timestamp ();
			double diff = now - p_begin_read;
			t_reads += double (acc_reads) * diff;
			p_begin_read = now;
			p_reads += (acc_reads--) ? diff : 0.0;
			read_mutex.unlock ();

			io_mutex.lock ();
			diff = now - p_begin_io;
			p_ios += (acc_ios--) ? diff : 0.0;
			p_begin_io = now;
			io_mutex.unlock ();
		}
	};

//! \}

__STXXL_END_NAMESPACE

#endif
