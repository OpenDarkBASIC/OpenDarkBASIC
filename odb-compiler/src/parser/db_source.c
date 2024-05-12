#include "odb-compiler/parser/db_source.h"
#include "odb-sdk/mfile.h"

int
db_source_open_file(struct db_source* s, struct ospathc filepath)
{
    struct mfile mf;

    /* FLEX expects to find an "EOB marker" at the end of its buffer, which is a
     * sequence of two NULL bytes. */
    if (mfile_map_cow_with_extra_padding(&mf, filepath, 2) != 0)
        return -1;

    ((char*)mf.address)[mf.size - 1] = '\0';
    ((char*)mf.address)[mf.size - 2] = '\0';

    s->text.data = (char*)mf.address;
    s->text.len = mf.size - 2; /* two EOB bytes -- also function as a null
                                  terminator */
    return 0;
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
