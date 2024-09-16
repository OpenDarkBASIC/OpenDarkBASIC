#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/utf8.h"
#include <errno.h>
#include <iconv.h>

int
utf16_to_utf8(struct utf8* out, struct utf16_view in)
{
    void*    new_mem;
    char*    inp;
    char*    outp;
    iconv_t  cd;
    size_t   nconv;
    size_t   inbytes = in.len * sizeof(uint16_t);
    size_t   outbytes = in.len * sizeof(char);
    utf8_idx outcapacity = in.len * sizeof(char);

    cd = iconv_open("UTF-8", "UTF-16");
    if (cd == (iconv_t)-1)
    {
        log_util_err(
            "iconv_open() failed in utf16_to_utf8(): %s\n", strerror(errno));
        goto iconv_open_failed;
    }

    new_mem = mem_realloc(out->data, outcapacity + 1);
    if (new_mem == NULL)
    {
        log_oom(outcapacity, "utf16_to_utf8()");
        goto alloc_outbuf_failed;
    }
    out->data = new_mem;

    inp = (char*)in.data;
    outp = (char*)out->data;
    while (inbytes)
    {
        nconv = iconv(cd, &inp, &inbytes, &outp, &outbytes);
        if (nconv == (size_t)-1)
        {
            switch (errno)
            {
                case E2BIG: {
                    void* new_mem = mem_realloc(out->data, outcapacity * 2 + 1);
                    if (new_mem == NULL)
                    {
                        log_oom(outcapacity * 2, "utf16_to_utf8()");
                        goto grow_outbuf_failed;
                    }
                    out->data = new_mem;
                    outp = (char*)out->data + outcapacity;
                    outbytes += outcapacity;
                    outcapacity *= 2;
                }
                break;

                case EINVAL:
                    inp++;
                    inbytes--;
                    break;

                default:
                    log_util_err(
                        "Failed to convert string to UTF-8: %s\n",
                        strerror(errno));
                    goto grow_outbuf_failed;
                    break;
            }
        }
    }

    out->len = outcapacity - outbytes;

    iconv_close(cd);
    return 0;

grow_outbuf_failed:
alloc_outbuf_failed:
    iconv_close(cd);
iconv_open_failed:
    return -1;
}

void
utf16_deinit(struct utf16 str)
{
    if (str.data)
        mem_free(str.data);
}

/*
FILE*
fopen_utf8_wb(const char* utf8_filename, int len)
{
    (void)len;
    return fopen(utf8_filename, "wb");
}

int
remove_utf8(const char* utf8_filename, int len)
{
    (void)len;
    return remove(utf8_filename);
}
*/
