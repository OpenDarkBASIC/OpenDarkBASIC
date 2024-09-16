#include "odb-util/fs.h"

/*
static int match_all(const char* path, void* user) { (void)path; (void)user;  return 1; }

struct on_list_strlist_ctx
{
    struct strlist* out;
    int (*match)(const char* name, void* user);
    void* user;

};
static int on_list_strlist(const char* name, void* user)
{
    struct on_list_strlist_ctx* ctx = user;
    if (ctx->match(name, ctx->user))
        if (strlist_add(ctx->out, cstr_view(name)) != 0)
            return -1;
    return 0;
}*/

/*
int
fs_list_strlist(struct strlist* out, struct str_view path)
{
    return fs_list_strlist_matching(out, path, match_all, NULL);
}

int
fs_list_strlist_matching(
    struct strlist* out,
    struct str_view path,
    int (*match)(const char* str, void* user),
    void* user)
{
    struct on_list_strlist_ctx ctx = { out, match, user };
    return fs_list(path, on_list_strlist, &ctx);
}
*/
