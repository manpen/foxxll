/***************************************************************************
 *  lib/io/create_file.cpp
 *
 *  Part of the STXXL. See http://stxxl.sourceforge.net
 *
 *  Copyright (C) 2002 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 *  Copyright (C) 2008, 2010 Andreas Beckmann <beckmann@cs.uni-frankfurt.de>
 *  Copyright (C) 2008, 2009 Johannes Singler <singler@ira.uka.de>
 *  Copyright (C) 2013 Timo Bingmann <tb@panthema.net>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#include <stxxl/bits/io/create_file.h>
#include <stxxl/bits/io/io.h>
#include <stxxl/bits/mng/config.h>
#include <stxxl/bits/common/error_handling.h>


__STXXL_BEGIN_NAMESPACE

file * create_file(const std::string & io_impl,
                   const std::string & filename,
                   int options, int physical_device_id, int disk_allocator_id)
{
    // construct temporary disk_config structure
    disk_config cfg(filename, 0, io_impl);
    cfg.queue = physical_device_id;
    cfg.direct = (options & file::DIRECT) ? 2 : 0;

    return create_file(cfg, options, disk_allocator_id);
}

file * create_file(const disk_config& cfg, int mode, int disk_allocator_id)
{
    // apply disk_config settings to open mode

    if (cfg.direct == 0)
        mode &= ~file::DIRECT;
    else if (cfg.direct == 1)
        mode |= file::DIRECT;
    else if (cfg.direct == 2) // set DIRECT for first try.
        mode |= file::DIRECT | file::TRY_DIRECT;

    // *** Select fileio Implementation

    if (cfg.io_impl == "syscall")
    {
        ufs_file_base * result =
            new syscall_file(cfg.path, mode, cfg.queue, disk_allocator_id);
        result->lock();

        if (cfg.unlink_on_open)
            result->unlink();

        return result;
    }
    else if (cfg.io_impl == "fileperblock_syscall")
    {
        fileperblock_file<syscall_file> * result =
            new fileperblock_file<syscall_file>(cfg.path, mode, cfg.queue, disk_allocator_id);
        result->lock();
        return result;
    }
    else if (cfg.io_impl == "memory")
    {
        mem_file * result = new mem_file(cfg.queue, disk_allocator_id);
        result->lock();
        return result;
    }
#if STXXL_HAVE_MMAP_FILE
    else if (cfg.io_impl == "mmap")
    {
        ufs_file_base * result =
            new mmap_file(cfg.path, mode, cfg.queue, disk_allocator_id);
        result->lock();

        if (cfg.unlink_on_open)
            result->unlink();

        return result;
    }
    else if (cfg.io_impl == "fileperblock_mmap")
    {
        fileperblock_file<mmap_file> * result =
            new fileperblock_file<mmap_file>(cfg.path, mode, cfg.queue, disk_allocator_id);
        result->lock();
        return result;
    }
#endif
#if STXXL_HAVE_SIMDISK_FILE
    else if (cfg.io_impl == "simdisk")
    {
        mode &= ~file::DIRECT;  // clear the DIRECT flag, this file is supposed to be on tmpfs
        ufs_file_base * result =
            new sim_disk_file(cfg.path, mode, cfg.queue, disk_allocator_id);
        result->lock();
        return result;
    }
#endif
#if STXXL_HAVE_WINCALL_FILE
    else if (cfg.io_impl == "wincall")
    {
        wfs_file_base * result =
            new wincall_file(cfg.path, mode, cfg.queue, disk_allocator_id);
        result->lock();
        return result;
    }
    else if (cfg.io_impl == "fileperblock_wincall")
    {
        fileperblock_file<wincall_file> * result =
            new fileperblock_file<wincall_file>(cfg.path, mode, cfg.queue, disk_allocator_id);
        result->lock();
        return result;
    }
#endif
#if STXXL_HAVE_BOOSTFD_FILE
    else if (cfg.io_impl == "boostfd")
    {
        boostfd_file * result =
            new boostfd_file(cfg.path, mode, cfg.queue, disk_allocator_id);
        result->lock();
        return result;
    }
    else if (cfg.io_impl == "fileperblock_boostfd")
    {
        fileperblock_file<boostfd_file> * result =
            new fileperblock_file<boostfd_file>(cfg.path, mode, cfg.queue, disk_allocator_id);
        result->lock();
        return result;
    }
#endif
#if STXXL_HAVE_WBTL_FILE
    else if (cfg.io_impl == "wbtl")
    {
        ufs_file_base * backend =
            new syscall_file(cfg.path, mode, -1, -1); // FIXME: ID
        wbtl_file * result =
            new stxxl::wbtl_file(backend, 16 * 1024 * 1024, 2, cfg.queue, disk_allocator_id);
        result->lock();

        if (cfg.unlink_on_open)
            backend->unlink();

        return result;
    }
#endif

    STXXL_THROW(std::runtime_error,
                "Unsupported disk I/O implementation '" << cfg.io_impl << "'.");
}

__STXXL_END_NAMESPACE
// vim: et:ts=4:sw=4
