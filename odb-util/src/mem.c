#include "odb-util/backtrace.h"
#include "odb-util/cli_colors.h"
#include "odb-util/hash.h"
#include "odb-util/hm.h"
#include "odb-util/mem.h"
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BACKTRACE_OMIT_COUNT 2

struct report_info
{
    uintptr_t location;
    mem_size  size;
#if defined(ODBUTIL_MEM_BACKTRACE)
    int    backtrace_size;
    char** backtrace;
#endif
};

HM_DECLARE_API_HASH(
    static,
    report,
    hash32,
    uintptr_t,
    struct report_info,
    32)
HM_DEFINE_API_HASH(
    report,
    hash32,
    uintptr_t,
    struct report_info,
    32,
    hash32_aligned_ptr)

struct state
{
    struct report* report;
    mem_size       allocations;
    mem_size       deallocations;
    unsigned       ignore_malloc : 1;
};

static ODBUTIL_THREADLOCAL struct state state;

int
mem_init(void)
{
    state.allocations = 0;
    state.deallocations = 0;

    report_init(&state.report);

    return 0;
}

/* ------------------------------------------------------------------------- */
#if defined(ODBUTIL_MEM_BACKTRACE)
static void
print_backtrace(void)
{
    char** bt;
    int    bt_size, i;

    if (state.ignore_malloc)
        return;

    if (!(bt = backtrace_get(&bt_size)))
    {
        fprintf(stderr, "Failed to generate backtrace\n");
        return;
    }

    for (i = BACKTRACE_OMIT_COUNT; i < bt_size; ++i)
    {
        if (strstr(bt[i], "invoke_main"))
            break;
        fprintf(stderr, "  %s\n", bt[i]);
    }
    backtrace_free(bt);
}
#else
#define print_backtrace()
#endif

/* ------------------------------------------------------------------------- */
static void
track_allocation(uintptr_t addr, mem_size size)
{
    struct report_info* info;
    ++state.allocations;

    if (size == 0)
    {
        fprintf(stderr, "malloc(0)\n");
#if defined(ODBUTIL_MEM_BACKTRACE)
        print_backtrace();
#endif
    }

    if (state.ignore_malloc)
        return;

    /* insert info into hashmap */
    state.ignore_malloc = 1;
    info = report_emplace_new(&state.report, addr);
    state.ignore_malloc = 0;
    if (info == NULL)
    {
        fprintf(stderr, 
            "Double allocation! This is usually caused by calling "
            "mem_track_allocation() on the same address twice.\n");
        print_backtrace();
        return;
    }

    /* record the location and size of the allocation */
    info->location = addr;
    info->size = size;

    /* Create backtrace to this allocation */
#if defined(ODBUTIL_MEM_BACKTRACE)
    state.ignore_malloc = 1;
    if (!(info->backtrace = backtrace_get(&info->backtrace_size)))
        fprintf(stderr, "Failed to generate backtrace\n");
    state.ignore_malloc = 0;
#endif
}

static void
track_deallocation(uintptr_t addr, const char* free_type)
{
    struct report_info* info;
    state.deallocations++;

    if (addr == 0)
    {
        fprintf(stderr, "free(NULL)\n");
#if defined(ODBUTIL_MEM_BACKTRACE)
        print_backtrace();
#endif
    }

    if (state.ignore_malloc)
        return;

    /* find matching allocation and remove from hashmap */
    info = report_erase(state.report, addr);
    if (info)
    {
#if defined(ODBUTIL_MEM_BACKTRACE)
        if (info->backtrace)
            backtrace_free(info->backtrace);
        else
            fprintf(stderr, "Allocation didn't have a backtrace (it was NULL)\n");
#endif
    }
    else
    {
        fprintf(stderr, "%s'ing something that was never allocated\n", free_type);
#if defined(ODBUTIL_MEM_BACKTRACE)
        print_backtrace();
#endif
    }
}

static void
acquire(uintptr_t addr, mem_size size)
{
    struct report_info* info;

    if (addr == 0)
        return;

    ++state.allocations;

    /* insert info into hashmap */
    state.ignore_malloc = 1;
    info = report_emplace_new(&state.report, addr);
    state.ignore_malloc = 0;
    if (info == NULL)
    {
        fprintf(stderr, 
            "Double allocation! This is usually caused by calling "
            "mem_acquire() on the same address twice.\n");
        print_backtrace();
        return;
    }

    /* record the location and size of the allocation */
    info->location = addr;
    info->size = size;

    /* Create backtrace to this allocation */
#if defined(ODBUTIL_MEM_BACKTRACE)
    state.ignore_malloc = 1;
    if (!(info->backtrace = backtrace_get(&info->backtrace_size)))
        fprintf(stderr, "Failed to generate backtrace\n");
    state.ignore_malloc = 0;
#endif
}

static mem_size
release(uintptr_t addr)
{
    struct report_info* info;

    if (addr == 0)
        return 0;

    state.deallocations++;

    /* find matching allocation and remove from hashmap */
    info = report_erase(state.report, addr);
    if (info)
    {
#if defined(ODBUTIL_MEM_BACKTRACE)
        if (info->backtrace)
            backtrace_free(info->backtrace);
        else
            fprintf(stderr, "Allocation didn't have a backtrace (it was NULL)\n");
#endif
        return info->size;
    }

    fprintf(stderr, "releasing something that was never allocated\n");
#if defined(ODBUTIL_MEM_BACKTRACE)
    print_backtrace();
#endif
    return 0;
}

/* ------------------------------------------------------------------------- */
void*
mem_alloc(mem_size size)
{
    void* p = malloc(size);
    if (p == NULL)
    {
        fprintf(stderr, "malloc() failed (out of memory)\n");
#if defined(ODBUTIL_MEM_BACKTRACE)
        print_backtrace(); /* probably won't work but may as well*/
#endif
        return NULL;
    }

    track_allocation((uintptr_t)p, size);
    return p;
}

/* ------------------------------------------------------------------------- */
void*
mem_realloc(void* p, mem_size new_size)
{
    uintptr_t old_addr = (uintptr_t)p;
    p = realloc(p, new_size);

    if (p == NULL)
    {
        fprintf(stderr, "realloc() failed (out of memory)\n");
#if defined(ODBUTIL_MEM_BACKTRACE)
        print_backtrace(); /* probably won't work but may as well*/
#endif
        return NULL;
    }

    if (old_addr)
        track_deallocation(old_addr, "realloc()");
    track_allocation((uintptr_t)p, new_size);

    return p;
}

/* ------------------------------------------------------------------------- */
void
mem_free(void* p)
{
    track_deallocation((uintptr_t)p, "free()");
    free(p);
}

/* ------------------------------------------------------------------------- */
static void
log_hex_ascii(const void* data, int len)
{
    int i;

    for (i = 0; i != 16; ++i)
        fprintf(stderr, "%c  ", "0123456789ABCDEF"[i]);
    putc(' ', stderr);
    for (i = 0; i != 16; ++i)
        putc("0123456789ABCDEF"[i], stderr);
    putc('\n', stderr);

    for (i = 0; i < len;)
    {
        int     j;
        uint8_t c = ((const uint8_t*)data)[i];
        for (j = 0; j != 16; ++j)
        {
            if (i + j < len)
                fprintf(stderr, "%02x ", c);
            else
                fprintf(stderr, "   ");
        }

        fprintf(stderr, " ");
        for (j = 0; j != 16 && i + j != len; ++j)
        {
            if (c >= 32 && c < 127) /* printable ascii */
                putc(c, stderr);
            else
                putc('.', stderr);
        }

        fprintf(stderr, "\n");
        i += 16;
    }
}

mem_size
mem_deinit(void)
{
    uintptr_t leaks;

    /* report details on any g_allocations that were not de-allocated */
    uintptr_t           addr;
    struct report_info* info;
    hm_for_each(state.report, addr, info)
    {
        fprintf(stderr, 
            "un-freed memory at 0x%" PRIx64 ", size 0x%" PRIx32 "\n",
            info->location,
            info->size);

#if defined(ODBUTIL_MEM_BACKTRACE)
        {
            int i;
            fprintf(stderr, "Backtrace:\n");
            for (i = BACKTRACE_OMIT_COUNT; i < info->backtrace_size; ++i)
            {
                if (strstr(info->backtrace[i], "invoke_main"))
                    break;
                fprintf(stderr, "  %s\n", info->backtrace[i]);
            }
        }
        backtrace_free(
            info->backtrace); /* this was allocated when malloc() was called */
#endif

#if defined(ODBUTIL_MEM_HEX_DUMP)
        if (info->size <= ODBUTIL_MEM_HEX_DUMP_SIZE)
            log_hex_ascii((void*)info->location, info->size);
#endif
    }

    state.ignore_malloc = 1;
    report_deinit(state.report);
    state.ignore_malloc = 0;

    /* overall report */
    leaks
        = (state.allocations > state.deallocations
               ? state.allocations - state.deallocations
               : state.deallocations - state.allocations);
    if (leaks)
    {
        fprintf(stderr, "Memory report:\n");
        fprintf(stderr, "  allocations   : %" PRIu32 "\n", state.allocations);
        fprintf(stderr, "  deallocations : %" PRIu32 "\n", state.deallocations);
        fprintf(stderr, FGB_RED "  memory leaks  : %" PRIu64 COL_RESET "\n", leaks);
    }

    return (mem_size)leaks;
}

void
mem_track_allocation(void* p)
{
    track_allocation((uintptr_t)p, 1);
}

void
mem_track_deallocation(void* p)
{
    track_deallocation((uintptr_t)p, "track_deallocation()");
}

void
mem_acquire(void* p, mem_size size)
{
    acquire((uintptr_t)p, size);
}

mem_size
mem_release(void* p)
{
    return release((uintptr_t)p);
}
