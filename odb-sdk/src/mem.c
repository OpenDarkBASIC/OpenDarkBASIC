#include "odb-sdk/backtrace.h"
#include "odb-sdk/hash.h"
#include "odb-sdk/hm.h"
#include "odb-sdk/log.h"
#include "odb-sdk/mem.h"
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BACKTRACE_OMIT_COUNT 2

struct state
{
    struct hm report;
    mem_size  allocations;
    mem_size  deallocations;
    mem_size  bytes_in_use;
    mem_size  bytes_in_use_peak;
    unsigned  ignore_malloc : 1;
};

struct report_info
{
    uintptr_t location;
    mem_size  size;
#if defined(ODBSDK_MEM_BACKTRACE)
    int    backtrace_size;
    char** backtrace;
#endif
};

static ODBSDK_THREADLOCAL struct state state;

/* ------------------------------------------------------------------------- */
int
mem_report_oom(mem_size bytes, const char* func_name)
{
    log_sdk_err("Failed to allocate %u bytes in %s\n", bytes, func_name);
    return -1;
}

/* ------------------------------------------------------------------------- */
static int
report_info_cmp(const void* a, const void* b, int size)
{
    return memcmp(a, b, (size_t)size);
}
int
mem_threadlocal_init(void)
{
    state.allocations = 0;
    state.deallocations = 0;
    state.bytes_in_use = 0;
    state.bytes_in_use_peak = 0;

    /*
     * Hashmap will call mem_alloc during init, need to ignore this to avoid
     * crashing.
     */
    state.ignore_malloc = 1;
    if (hm_init_with_options(
            &state.report,
            sizeof(uintptr_t),
            sizeof(struct report_info),
            4096,
            hash32_ptr,
            report_info_cmp)
        != 0)
    {
        return -1;
    }
    state.ignore_malloc = 0;

    return 0;
}

/* ------------------------------------------------------------------------- */
#if defined(ODBSDK_MEM_BACKTRACE)
static void
print_backtrace(void)
{
    char** bt;
    int    bt_size, i;

    if (state.ignore_malloc)
        return;

    if (!(bt = backtrace_get(&bt_size)))
    {
        log_sdk_warn("Failed to generate backtrace\n");
        return;
    }

    for (i = BACKTRACE_OMIT_COUNT; i < bt_size; ++i)
    {
        if (strstr(bt[i], "invoke_main"))
            break;
        log_sdk_warn("  %s\n", bt[i]);
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
        log_sdk_warn("malloc(0)\n");
#if defined(ODBSDK_MEM_BACKTRACE)
        print_backtrace();
#endif
    }

    if (state.ignore_malloc)
        return;

    /*
     * Record allocation info. Call to hashmap and backtrace_get() may allocate
     * memory, so set flag to ignore the call to malloc() when inserting.
     */
    state.bytes_in_use += size;
    if (state.bytes_in_use_peak < state.bytes_in_use)
        state.bytes_in_use_peak = state.bytes_in_use;

    state.ignore_malloc = 1;
    /* insert info into hashmap */
    switch (hm_insert(&state.report, &addr, (void**)&info))
    {
        case 1: break;
        case 0:
            log_sdk_err(
                "Double allocation! This is usually caused by calling "
                "mem_track_allocation() on the same address twice.\n");
            print_backtrace();
            break;
        default:
            log_sdk_err(
                "Hashmap insert failed! Expect to see incorrect memory leak "
                "reports!\n");
    }

    /* record the location and size of the allocation */
    info->location = addr;
    info->size = size;

    /* Create backtrace to this allocation */
#if defined(ODBSDK_MEM_BACKTRACE)
    if (!(info->backtrace = backtrace_get(&info->backtrace_size)))
        log_sdk_warn("Failed to generate backtrace\n");
#endif
    state.ignore_malloc = 0;

    return;
}

static void
track_deallocation(uintptr_t addr, const char* free_type)
{
    struct report_info* info;
    state.deallocations++;

    if (addr == 0)
    {
        log_sdk_warn("free(NULL)\n");
#if defined(ODBSDK_MEM_BACKTRACE)
        print_backtrace();
#endif
    }

    if (state.ignore_malloc)
        return;

    /* find matching allocation and remove from hashmap */
    info = hm_erase(&state.report, &addr);
    if (info)
    {
        state.bytes_in_use -= info->size;
#if defined(ODBSDK_MEM_BACKTRACE)
        if (info->backtrace)
            backtrace_free(info->backtrace);
        else
            log_sdk_warn("Allocation didn't have a backtrace (it was NULL)\n");
#endif
    }
    else
    {
        log_sdk_warn("%s'ing something that was never allocated\n", free_type);
#if defined(ODBSDK_MEM_BACKTRACE)
        print_backtrace();
#endif
    }
}

/* ------------------------------------------------------------------------- */
void*
mem_alloc(mem_size size)
{
    void* p = malloc(size);
    if (p == NULL)
    {
        log_sdk_err("malloc() failed (out of memory)\n");
#if defined(ODBSDK_MEM_BACKTRACE)
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
        log_sdk_err("realloc() failed (out of memory)\n");
#if defined(ODBSDK_MEM_BACKTRACE)
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
mem_size
mem_threadlocal_deinit(void)
{
    uintptr_t leaks;

    --state.allocations; /* this is the single allocation still held by the
                            report hashmap */

    log_sdk_note("Memory report:\n");

    /* report details on any g_allocations that were not de-allocated */
    HM_FOR_EACH(&state.report, void*, struct report_info, key, info)
    log_sdk_err(
        "un-freed memory at 0x%" PRIx64 ", size 0x%" PRIx32 "\n",
        info->location,
        info->size);

#if defined(ODBSDK_MEM_BACKTRACE)
    {
        int i;
        log_sdk_note("Backtrace:\n");
        for (i = BACKTRACE_OMIT_COUNT; i < info->backtrace_size; ++i)
        {
            if (strstr(info->backtrace[i], "invoke_main"))
                break;
            log_raw("", "", "  %s\n", info->backtrace[i]);
        }
    }
    backtrace_free(
        info->backtrace); /* this was allocated when malloc() was called */
#endif
#if defined(ODBSDK_MEM_HEX_DUMP)
    if (info->size <= ODBSDK_MEM_HEX_DUMP_SIZE)
    {
        intptr_t i;
        uint8_t*    p = (void*)info->location;
        log_sdk_note("Hex Dump:\n");

        log_raw("", "", "  ");
        for (i = 0; i != 16; ++i)
            log_raw("", "", "%c  ", "0123456789ABCDEF"[i]);
        log_raw("", "", " ");
        for (i = 0; i != 16; ++i)
            log_raw("", "", "%c", "0123456789ABCDEF"[i]);
        log_raw("", "", "\n");

        for (i = 0; i < info->size; )
        {
            int j;
            log_raw("", "", "  ");
            for (j = 0; j != 16; ++j)
            {
                if (i + j < info->size)
                    log_raw("", "", "%02x ", p[i]);
                else
                    log_raw("", "", "   ");
            }

            log_raw("", "", " ");
            for (j = 0; j != 16 && i + j != info->size; ++j)
            {
                if (p[i] >= 32 && p[i] < 127)  /* printable ascii */
                    log_raw("", "", "%c", p[i]);
                else
                    log_raw("", "", ".");
            }

            log_raw("", "", "\n");
            i += 16;
        }
    }
#endif
    HM_END_EACH

    /* overall report */
    leaks
        = (state.allocations > state.deallocations
               ? state.allocations - state.deallocations
               : state.deallocations - state.allocations);
    log_sdk_note("  allocations   : %" PRIu32 "\n", state.allocations);
    log_sdk_note("  deallocations : %" PRIu32 "\n", state.deallocations);
    log_sdk_note("  memory leaks  : %" PRIu64 "\n", leaks);
    log_sdk_note(
        "  peak memory   : %" PRIu32 " bytes\n", state.bytes_in_use_peak);

    ++state.allocations; /* this is the single allocation still held by the
                            report hashmap */
    state.ignore_malloc = 1;
    hm_deinit(&state.report);
    state.ignore_malloc = 0;

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
