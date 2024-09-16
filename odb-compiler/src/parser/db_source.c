#include "odb-compiler/parser/db_source.h"
#include "odb-util/mfile.h"
#include "odb-util/mstream.h"
#include <stdio.h>

int
db_source_open_file(struct db_source* s, struct ospathc filepath)
{
    struct mfile mf_orig;
    struct mfile mf_flex;

    /* FLEX expects to find an "EOB marker" at the end of its buffer, which is a
     * sequence of two NULL bytes. */
    if (mfile_map_read(&mf_orig, filepath, 1) != 0)
        goto map_file_failed;
    if (mfile_map_mem(&mf_flex, mf_orig.size + 2) != 0)
        goto map_flex_failed;

    memcpy(mf_flex.address, mf_orig.address, mf_orig.size);
    ((char*)mf_flex.address)[mf_flex.size - 1] = '\0';
    ((char*)mf_flex.address)[mf_flex.size - 2] = '\0';

    s->text.data = (char*)mf_flex.address;
    s->text.len = mf_flex.size - 2; /* two EOB bytes -- also function as a null
                                       terminator */

    mfile_unmap(&mf_orig);
    return 0;

map_flex_failed:
    mfile_unmap(&mf_orig);
map_file_failed:
    return -1;
}

int
db_source_open_string(struct db_source* s, struct utf8_view str)
{
    struct mfile mf;
    if (mfile_map_mem(&mf, str.len + 2) != 0)
        return -1;

    memcpy(mf.address, str.data, str.len);
    ((char*)mf.address)[mf.size - 1] = '\0';
    ((char*)mf.address)[mf.size - 2] = '\0';

    s->text.data = (char*)mf.address;
    s->text.len = mf.size - 2; /* two EOB bytes -- also function as a null
                                  terminator */
    return 0;
}

int
db_source_open_stream(struct db_source* s, FILE* fd)
{
    int            c;
    struct mstream ms;
    struct mfile   mf;

    ms = mstream_init_writable();
    while ((c = getc(fd)) != EOF)
        if (mstream_write(&ms, &c, 1) != 0)
            goto read_failed;

    if (mfile_map_mem(&mf, ms.ptr + 2) != 0)
        goto map_failed;

    memcpy(mf.address, ms.data, ms.ptr);
    ((char*)mf.address)[mf.size - 1] = '\0';
    ((char*)mf.address)[mf.size - 2] = '\0';

    s->text.data = (char*)mf.address;
    s->text.len = mf.size - 2; /* two EOB bytes -- also function as a null
                                  terminator */

    mstream_free_writable(&ms);
    return 0;

map_failed:
read_failed:
    mstream_free_writable(&ms);
    return -1;
}

int
db_source_ref_string(struct db_source* s, struct utf8* str)
{
    /* FLEX expects to find an "EOB marker" at the end of its buffer, which is a
     * sequence of two NULL bytes. */
    struct utf8_view eob = {"", 0, 1};
    if (utf8_append(str, eob) != 0)
        return -1;

    s->text.data = str->data;
    s->text.len = str->len;

    return 0;
}

void
db_source_close(struct db_source* s)
{
    struct mfile mf = {(void*)s->text.data, s->text.len + 2};
    mfile_unmap(&mf);
}
