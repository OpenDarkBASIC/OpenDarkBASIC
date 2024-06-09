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

struct report_info
{
    uintptr_t location;
    mem_size  size;
#if defined(ODBSDK_MEM_BACKTRACE)
    int    backtrace_size;
    char** backtrace;
#endif
};
struct report_kvs
{
    uintptr_t*          keys;
    struct report_info* values;
};

static int
report_kvs_alloc(struct report_kvs* kvs, int32_t capacity)
{
    if ((kvs->keys = mem_alloc(sizeof(uintptr_t) * capacity)) == NULL)
        return -1;
    if ((kvs->values = mem_alloc(sizeof(struct report_info) * capacity))
        == NULL)
    {
        mem_free(kvs->keys);
        return -1;
    }

    return 0;
}
static void
report_kvs_free(struct report_kvs* kvs)
{
    mem_free(kvs->values);
    mem_free(kvs->keys);
}
static uintptr_t
report_kvs_get_key(const struct report_kvs* kvs, int32_t slot)
{
    return kvs->keys[slot];
}
static void
report_kvs_set_key(struct report_kvs* kvs, int32_t slot, uintptr_t key)
{
    kvs->keys[slot] = key;
}
static int
report_kvs_keys_equal(uintptr_t k1, uintptr_t k2)
{
    return k1 == k2;
}
static struct report_info*
report_kvs_get_value(struct report_kvs* kvs, int32_t slot)
{
    return &kvs->values[slot];
}
static void
report_kvs_set_value(
    struct report_kvs* kvs, int32_t slot, struct report_info* value)
{
    kvs->values[slot] = *value;
}

HM_DECLARE_API_FULL(
    report,
    hash32,
    uintptr_t,
    struct report_info,
    32,
    static,
    struct report_kvs)
HM_DEFINE_API_FULL(
    report,
    hash32,
    uintptr_t,
    struct report_info,
    32,
    hash32_aligned_ptr,
    report_kvs_alloc,
    report_kvs_free,
    report_kvs_get_key,
    report_kvs_set_key,
    report_kvs_keys_equal,
    report_kvs_get_value,
    report_kvs_set_value,
    128,
    70)

struct state
{
    struct report* report;
    mem_size       allocations;
    mem_size       deallocations;
    mem_size       bytes_in_use;
    mem_size       bytes_in_use_peak;
    unsigned       ignore_malloc : 1;
};

static struct state state;

int
mem_init(void)
{
    state.allocations = 0;
    state.deallocations = 0;
    state.bytes_in_use = 0;
    state.bytes_in_use_peak = 0;

    report_init(&state.report);

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

    state.bytes_in_use += size;
    if (state.bytes_in_use_peak < state.bytes_in_use)
        state.bytes_in_use_peak = state.bytes_in_use;

    /* insert info into hashmap */
    state.ignore_malloc = 1;
    info = report_emplace_new(&state.report, addr);
    state.ignore_malloc = 0;
    if (info == NULL)
    {
        log_sdk_err(
            "Double allocation! This is usually caused by calling "
            "mem_track_allocation() on the same address twice.\n");
        print_backtrace();
        return;
    }

    /* record the location and size of the allocation */
    info->location = addr;
    info->size = size;

    /* Create backtrace to this allocation */
#if defined(ODBSDK_MEM_BACKTRACE)
    state.ignore_malloc = 1;
    if (!(info->backtrace = backtrace_get(&info->backtrace_size)))
        log_sdk_warn("Failed to generate backtrace\n");
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
        log_sdk_warn("free(NULL)\n");
#if defined(ODBSDK_MEM_BACKTRACE)
        print_backtrace();
#endif
    }

    if (state.ignore_malloc)
        return;

    /* find matching allocation and remove from hashmap */
    info = report_erase(state.report, addr);
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
mem_deinit(void)
{
    uintptr_t leaks;

    --state.allocations; /* this is the single allocation still held by the
                            report hashmap */

    /* report details on any g_allocations that were not de-allocated */
    uintptr_t           addr;
    struct report_info* info;
    hm_for_each(state.report, addr, info)
    {
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
                log_raw("  %s\n", info->backtrace[i]);
            }
        }
        backtrace_free(
            info->backtrace); /* this was allocated when malloc() was called */
#endif

#if defined(ODBSDK_MEM_HEX_DUMP)
        if (info->size <= ODBSDK_MEM_HEX_DUMP_SIZE)
        {
            intptr_t i;
            uint8_t* p = (void*)info->location;
            log_sdk_note("Hex Dump:\n");

            log_raw("  ");
            for (i = 0; i != 16; ++i)
                log_raw("%c  ", "0123456789ABCDEF"[i]);
            log_raw(" ");
            for (i = 0; i != 16; ++i)
                log_raw("%c", "0123456789ABCDEF"[i]);
            log_raw("\n");

            for (i = 0; i < info->size;)
            {
                int j;
                log_raw("  ");
                for (j = 0; j != 16; ++j)
                {
                    if (i + j < info->size)
                        log_raw("%02x ", p[i]);
                    else
                        log_raw("   ");
                }

                log_raw(" ");
                for (j = 0; j != 16 && i + j != info->size; ++j)
                {
                    if (p[i] >= 32 && p[i] < 127) /* printable ascii */
                        log_raw("%c", p[i]);
                    else
                        log_raw(".");
                }

                log_raw("\n");
                i += 16;
            }
        }
#endif
    }

    /* overall report */
    log_sdk_note("Memory report:\n");
    leaks
        = (state.allocations > state.deallocations
               ? state.allocations - state.deallocations
               : state.deallocations - state.allocations);
    log_raw("  allocations   : %" PRIu32 "\n", state.allocations);
    log_raw("  deallocations : %" PRIu32 "\n", state.deallocations);
    if (leaks)
        log_raw("  {e:memory leaks  : %" PRIu64 "}\n", leaks);
    else
        log_raw("  memory leaks  : %" PRIu64 "\n", leaks);
    log_raw("  peak memory   : %" PRIu32 " bytes\n", state.bytes_in_use_peak);

    ++state.allocations; /* this is the single allocation still held by the
                            report hashmap */
    state.ignore_malloc = 1;
    report_deinit(state.report);
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
