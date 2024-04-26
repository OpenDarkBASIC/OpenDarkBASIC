#include "odb-sdk/mstream.h"
#include "odb-sdk/str.h"

int
mstream_read_string_until_delim(struct mstream* ms, char delim, struct str_view* str)
{
    const char* data = ms->address;
    str->data = &data[ms->idx];
    str->len = 0;
    for (; ms->idx + str->len != ms->size; ++str->len)
        if (str->data[str->len] == delim)
        {
            ms->idx += str->len + 1;
            return 0;
        }

    return -1;
}

int
mstream_read_string_until_condition(struct mstream* ms, int (*cond)(char), struct str_view* str)
{
    const char* data = ms->address;
    str->data = &data[ms->idx];
    str->len = 0;
    for (; ms->idx + str->len != ms->size; ++str->len)
        if (cond(str->data[str->len]))
        {
            ms->idx += str->len + 1;
            while (!mstream_at_end(ms) && cond(data[ms->idx]))
                ++ms->idx;
            return 0;
        }

    return -1;
}
