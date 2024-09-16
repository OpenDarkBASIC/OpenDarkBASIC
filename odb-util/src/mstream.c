#include "odb-util/mem.h"
#include "odb-util/mstream.h"

void
mstream_free_writable(struct mstream* ms)
{
    if (ms->data)
        mem_free(ms->data);
}

int
mstream_grow(struct mstream* ms, int additional_size)
{
    while (ms->capacity < ms->ptr + additional_size)
    {
        int   new_cap = ms->capacity == 0 ? 32 : ms->capacity * 2;
        void* new_data = mem_realloc(ms->data, new_cap);
        if (new_data == NULL)
        {
            ms->error = 1;
            return -1;
        }
        ms->data = new_data;
        ms->capacity = new_cap;
    }

    return 0;
}
