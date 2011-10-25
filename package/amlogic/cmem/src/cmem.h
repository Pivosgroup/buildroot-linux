/*
 *  Copyright 2010 by AMLOGIC INC.
 *  Common physical memory allocator driver.
 */

/*
 * Copyright (c) 2009, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * @file    cmem.h
 * @brief   Describes the interface to the contiguous memory allocator.
 *
 * The cmem user interface library wraps file system calls to an associated
 * kernel module (cmemk.ko), which needs to be loaded in order for calls to
 * this library to succeed.
 *
 * The following is an example of installing the cmem kernel module:
 * 
 * @verbatim /sbin/insmod cmemk.ko pools=4x30000,2x500000 phys_start=0x0 phys_end=0x3000000 @endverbatim
 *     - phys_start and phys_end must be specified in hexadecimal format
 *     - phys_start is "inclusive" and phys_end is "exclusive", i.e.,
 *       phys_end should be "end address + 1".
 *     - pools must be specified using decimal format (for both number and
 *       size), since using hexadecimal format would visually clutter the
 *       specification due to the use of "x" as a token separator
 *     - it's possible to insmod cmemk.ko without specifying any memory blocks,
 *       in which case CMEM_getPhys() and CMEM_cache*() APIs can still be
 *       utilized by an application.
 *
 * This particular command creates 2 pools. The first pool is created with 4
 * buffers of size 30000 bytes and the second pool is created with 2 buffers
 * of size 500000 bytes. The CMEM pool buffers start at 0x0 and end at
 * 0x2FFFFFF (max). 
 *
 * There is also support for a 2nd contiguous memory block to be specified,
 * with all the same features supported for the 2nd block as with the 1st.
 * This 2nd block is specified with *_1 parameters.  The following example
 * expands upon the first example above:
 * 
 * @verbatim /sbin/insmod cmemk.ko pools=4x30000,2x500000 phys_start=0x0 phys_end=0x3000000
    pools_1=4x65536 phys_start_1=0x80000000 phys_end_1=0x80010000 @endverbatim
 *
 * This particular command, in addition to the pools explained above,
 * creates 1 pool (with 4 buffers of size 64KB) in a 2nd memory block which
 * starts at 0x80000000 and ends at 0x8000FFFF (specified as "end + 1" on the
 * insmod command).
 *
 * In order to access this 2nd memory block, new APIs have been added to
 * CMEM which allow specification of the block ID.
 *
 * Pool buffers are aligned on a module-dependent boundary, and their sizes are
 * rounded up to this same boundary.  This applies to each buffer within a
 * pool.  The total space used by an individual pool will therefore be greater
 * than (or equal to) the exact amount requested in the installation of the
 * module.
 *
 * The poolid used in the driver calls would be 0 for the first pool and 1 for
 * the second pool.
 *
 * Pool allocations can be requested explicitly by pool number, or more
 * generally by just a size.  For size-based allocations, the pool which best
 * fits the requested size is automatically chosen.  Some CMEM APIs (newer
 * ones) accept a blockid as a parameter, in order to specify which of the
 * multiple blocks to operate on.  For 'legacy' APIs (ones that existed before
 * the support for multiple blocks) where a blockid is still needed, block 0
 * is assumed.
 *
 * There is also support for a general purpose heap.  In addition to the 2
 * pools described above, a general purpose heap block is created from which
 * allocations of any size can be requested.  Internally, allocation sizes are
 * rounded up to a module-dependent boundary and allocation addresses are
 * aligned either to this same boundary or to the requested alignment
 * (whichever is greater).
 *
 * The size of the heap block is the amount of CMEM memory remaining after all
 * pool allocations.  If more heap space is needed than is available after pool
 * allocations, you must reduce the amount of CMEM memory granted to the pools.
 *
 * Buffer allocation is tracked at the file descriptor level by way of a
 * 'registration' list.  The initial allocator of a buffer (the process that
 * calls CMEM_alloc()) is automatically added to the registration list,
 * and further processes can become registered for the same buffer by way
 * of the CMEM_registerAlloc() API (and unregister with the
 * CMEM_unregister() API).  This registration list for each buffer
 * allows for buffer ownership tracking and cleanup on a
 * per-file-descriptor basis, so that when a process exits or dies without
 * having explicitly freed/unregistered its buffers, they get automatically
 * unregistered (and freed when no more registered file descriptors exist).
 * Only when the last registered file descriptor frees a buffer (either
 * explictily, or by auto-cleanup) does a buffer actually get freed back to
 * the kernel module.
 *
 * Since the CMEM interface library doesn't use the GT tracing facility, there
 * is one configuration option available for the CMEM module to control
 * whether the debug or release interface library is used for building the
 * application.  This config parameter is named 'debug' and is of type bool,
 * and the default value is 'false'.
 *
 * The following line is an example of enabling usage of the debug interface
 * library:
 *     var cmem = xdc.useModule('ti.sdo.linuxutils.cmem.CMEM');
 *     cmem.debug = true;
 * This will enable "CMEM Debug" statements to be printed to stdout.
 */
/**
 *  @defgroup   ti_sdo_linuxutils_cmem_CMEM  Contiguous Memory Manager
 *
 *  This is the API for the Contiguous Memory Manager.
 */

#ifndef CMEM_H
#define CMEM_H

#if defined (__cplusplus)
extern "C" {
#endif

/** @ingroup    CMEM */
/*@{*/

#define CMEM_VERSION    0x03000000U

/* ioctl cmd "flavors" */
#define CMEM_WB                         0x00010000
#define CMEM_INV                        0x00020000
#define CMEM_HEAP                       0x00040000  /**< operation applies to heap */
#define CMEM_POOL                       0x00000000  /**< operation applies to a pool */
#define CMEM_CACHED                     0x00080000  /**< allocated buffer is cached */
#define CMEM_NONCACHED                  0x00000000  /**< allocated buffer is not cached */
#define CMEM_PHYS                       0x00100000

#define CMEM_IOCMAGIC                   0x0000fe00

/* supported "base" ioctl cmds for the driver. */
#define CMEM_IOCALLOC                   1
#define CMEM_IOCALLOCHEAP               2
#define CMEM_IOCFREE                    3
#define CMEM_IOCGETPHYS                 4
#define CMEM_IOCGETSIZE                 5
#define CMEM_IOCGETPOOL                 6
#define CMEM_IOCCACHE                   7
#define CMEM_IOCGETVERSION              8
#define CMEM_IOCGETBLOCK                9
#define CMEM_IOCREGUSER                 10
/*
 * New ioctl cmds should use integers greater than the largest current cmd
 * in order to not break backward compatibility.
 */

/* supported "flavors" to "base" ioctl cmds for the driver. */
#define CMEM_IOCCACHEWBINV              CMEM_IOCCACHE | CMEM_WB | CMEM_INV
#define CMEM_IOCCACHEWB                 CMEM_IOCCACHE | CMEM_WB
#define CMEM_IOCCACHEINV                CMEM_IOCCACHE | CMEM_INV
#define CMEM_IOCALLOCCACHED             CMEM_IOCALLOC | CMEM_CACHED
#define CMEM_IOCALLOCHEAPCACHED         CMEM_IOCALLOCHEAP | CMEM_CACHED
#define CMEM_IOCFREEHEAP                CMEM_IOCFREE | CMEM_HEAP
#define CMEM_IOCFREEPHYS                CMEM_IOCFREE | CMEM_PHYS
#define CMEM_IOCFREEHEAPPHYS            CMEM_IOCFREE | CMEM_HEAP | CMEM_PHYS

#define CMEM_IOCCMDMASK                 0x000000ff

/**
 * @brief Parameters for CMEM_alloc(), CMEM_allocPool(), CMEM_free().
 */
typedef struct CMEM_AllocParams {
    int type;           /**< either CMEM_HEAP or CMEM_POOL */
    int flags;          /**< either CMEM_CACHED or CMEM_NONCACHED */
    size_t alignment;   /**<
                         * only used for heap allocations, must be power of 2
                         */
} CMEM_AllocParams;

extern CMEM_AllocParams CMEM_DEFAULTPARAMS;

typedef struct CMEM_BlockAttrs {
    unsigned long phys_base;
    size_t size;
} CMEM_BlockAttrs;

/** @cond INTERNAL */

/**
 */
union CMEM_AllocUnion {
    struct {                    /**< */
        size_t size;
        size_t align;
        int blockid;
    } alloc_heap_inparams;      /**< */
    struct {                    /**< */
        int poolid;
        int blockid;
    } alloc_pool_inparams;      /**< */
    struct {                    /**< */
        int poolid;
        int blockid;
    } get_size_inparams;        /**< */
    struct {                    /**< */
        size_t size;
        int blockid;
    } get_pool_inparams;        /**< */
    struct {                    /**< */
        unsigned long physp;
        size_t size;
    } alloc_pool_outparams;     /**< */
    struct {                    /**< */
        unsigned long physp;
        size_t size;
    } get_block_outparams;      /**< */
    struct {                    /**< */
        int poolid;
        size_t size;
    } free_outparams;           /**< */
    unsigned long physp;
    unsigned long virtp;
    size_t size;
    int poolid;
    int blockid;
};

/** @endcond */

/**
 * @brief Initialize the CMEM module. Must be called before other API calls.
 *
 * @return 0 for success or -1 for failure.
 *
 * @sa CMEM_exit
 */
int CMEM_init(void);

/**
 * @brief Find the pool in memory block blockid that best fits a given
 * buffer size and has a buffer available.
 *
 * @param   blockid  Block number
 * @param   size     The buffer size for which a pool is needed.
 *
 * @return A poolid that can be passed to CMEM_allocPool(), or -1 for error.
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_allocPool()
 * @sa CMEM_free()
 */
int CMEM_getPool(int blockid, size_t size);

/**
 * @brief Allocate memory from a specified pool in a specified memory block.
 *
 * @param   blockid The memory block from which to allocate.
 * @param   poolid  The pool from which to allocate memory.
 * @param   params  Allocation parameters.
 *
 * @remarks @c params->type is ignored - a pool will always be used.
 * @remarks @c params->alignment is unused, since pool buffers are already
 *          aligned to specific boundaries.
 *
 * @return A pointer to the allocated buffer, or NULL for failure.
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_alloc()
 * @sa CMEM_registerAlloc()
 * @sa CMEM_unregister()
 * @sa CMEM_free()
 */
void *CMEM_allocPool(int blockid, int poolid, CMEM_AllocParams *params);

/**
 * @brief Allocate memory of a specified size from a specified memory block
 *
 * @param   blockid The memory block from which to allocate.
 * @param   size    The size of the buffer to allocate.
 * @param   params  Allocation parameters.
 *
 * @remarks Used to allocate memory from either a pool or the heap.
 *          If doing a pool allocation, the pool that best fits the requested
 *          size will be selected.  Use CMEM_allocPool() to allocate from a
 *          specific pool.
 *
 * @remarks Allocation will be cached or noncached, as specified by params.
 *          params->alignment valid only for heap allocation.
 *
 * @return A pointer to the allocated buffer, or NULL for failure.
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_allocPool()
 * @sa CMEM_registerAlloc()
 * @sa CMEM_unregister()
 * @sa CMEM_free()
 */
void *CMEM_alloc(int blockid, size_t size, CMEM_AllocParams *params);

/**
 * @brief Register shared usage of an already-allocated buffer
 *
 * @param   physp  Physical address of the already-allocated buffer.
 *
 * @remarks Used to register the calling process for usage of an
 *          already-allocated buffer, for the purpose of shared usage of
 *          the buffer.
 *
 * @remarks Allocation properties (such as cached/noncached or heap/pool)
 *          are inherited from original allocation call.
 *
 * @return  A process-specific pointer to the allocated buffer, or NULL
 *          for failure.
 *
 * @pre Must have called some form of CMEM_alloc*()
 *
 * @sa CMEM_allocPool()
 * @sa CMEM_alloc()
 * @sa CMEM_free()
 * @sa CMEM_unregister()
 */
void *CMEM_registerAlloc(unsigned long physp);

/**
 * @brief Free a buffer previously allocated with
 *        CMEM_alloc()/CMEM_allocPool().
 *
 * @param   ptr     The pointer to the buffer.
 * @param   params  Allocation parameters.
 *
 * @remarks Use the same CMEM_AllocParams as was used for the allocation.
 *          params->flags is "don't care".  params->alignment is "don't
 *          care".
 *
 * @return 0 for success or -1 for failure.
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_alloc()
 * @sa CMEM_allocPool()
 * @sa CMEM_registerAlloc()
 * @sa CMEM_unregister()
 */
int CMEM_free(void *ptr, CMEM_AllocParams *params);

/**
 * @brief Unregister use of a buffer previously registered with
 *        CMEM_registerAlloc().
 *
 * @param   ptr     The pointer to the buffer.
 * @param   params  Allocation parameters.
 *
 * @remarks Use the same CMEM_AllocParams as was used for the allocation.
 *          params->flags is "don't care".  params->alignment is "don't
 *          care".
 *
 * @return 0 for success or -1 for failure.
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_alloc()
 * @sa CMEM_allocPool()
 * @sa CMEM_registerAlloc()
 * @sa CMEM_free()
 */
int CMEM_unregister(void *ptr, CMEM_AllocParams *params);

/**
 * @brief Get the physical address of a contiguous buffer.
 *
 * @param   ptr     The pointer to the buffer.
 *
 * @return The physical address of the buffer or 0 for failure.
 *
 * @pre Must have called CMEM_init()
 */
unsigned long CMEM_getPhys(void *ptr);

/**
 * @brief Do a cache writeback of the block pointed to by @c ptr/@c size
 *
 * @param   ptr     Pointer to block to writeback
 * @param   size    Size in bytes of block to writeback.
 *
 * @return Success/failure boolean value
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_cacheInv()
 * @sa CMEM_cacheWbInv()
 */
int CMEM_cacheWb(void *ptr, size_t size);

/**
 * @brief Do a cache invalidate of the block pointed to by @c ptr/@c size
 *
 * @param   ptr     Pointer to block to invalidate
 * @param   size    Size in bytes of block to invalidate
 *
 * @return Success/failure boolean value
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_cacheWb()
 * @sa CMEM_cacheWbInv()
 */
int CMEM_cacheInv(void *ptr, size_t size);

/**
 * @brief Do a cache writeback/invalidate of the block pointed to by
 *        @c ptr/@c size
 *
 * @param   ptr     Pointer to block to writeback/invalidate
 * @param   size    Size in bytes of block to writeback/invalidate
 *
 * @return Success/failure boolean value
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_cacheInv()
 * @sa CMEM_cacheWb()
 */
int CMEM_cacheWbInv(void *ptr, size_t size);

/**
 * @brief Retrieve version from CMEM driver.
 *
 * @return Installed CMEM driver's version number.
 *
 * @pre Must have called CMEM_init()
 */
int CMEM_getVersion(void);

/**
 * @brief Retrieve extended memory block attributes from CMEM driver
 *
 * @param   blockid      Block number
 * @param   pattrs       Pointer to CMEM_BlockAttrs struct
 *
 * @return Success (0) or failure (-1).
 *
 * @remarks Currently this API returns the same values as CMEM_getBlock().
 *
 * @pre Must have called CMEM_init()
 *
 * @sa  CMEM_getBlock()
 */
int CMEM_getBlockAttrs(int blockid, CMEM_BlockAttrs *pattrs);

/**
 * @brief Finalize the CMEM module.
 *
 * @return 0 for success or -1 for failure.
 *
 * @remarks After this function has been called, no other CMEM function may be
 *          called (unless CMEM is reinitialized).
 *
 * @pre Must have called CMEM_init()
 *
 * @sa CMEM_init()
 */
int CMEM_exit(void);

/*@}*/

#if defined (__cplusplus)
}
#endif

#endif
