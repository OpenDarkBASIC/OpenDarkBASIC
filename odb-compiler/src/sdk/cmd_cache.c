#include "odb-compiler/sdk/cmd_cache.h"
#include "odb-sdk/fs.h"
#include "odb-sdk/mfile.h"
#include "odb-sdk/mstream.h"
#include "odb-sdk/utf8.h"

#define VERSION 0

int
cmd_cache_load(
    struct plugin_ids**       cached_plugins,
    const struct plugin_list* plugins,
    struct cmd_list*          cmds,
    enum sdk_type             sdk_type,
    enum target_arch          arch,
    enum target_platform      platform)
{
    plugin_id          cached_plugin_id;
    plugin_id          cached_plugin_count;
    cmd_id             cached_cmd;
    cmd_id             cached_cmd_count;
    struct plugin_ids* cached_plugin_map;
    struct mfile       mf;
    struct mstream     ms;
    struct utf8        fname = empty_utf8();
    struct ospath      path = empty_ospath();
    plugin_ids_init(&cached_plugin_map);

    if (fs_get_appdata_dir(&path) != 0)
        goto error;

    if (utf8_fmt(
            &fname,
            "%s-%s-%s.dat",
            sdk_type_to_name(sdk_type),
            target_arch_to_name(arch),
            target_platform_to_name(platform))
        != 0)
    {
        goto error;
    }

    if (ospath_join_cstr(&path, "cmd-cache") != 0)
        goto error;
    if (fs_make_path(path) != 0)
        goto error;
    if (ospath_join(&path, utf8_ospathc(fname)) != 0)
        goto error;

    if (mfile_map_read(&mf, ospathc(path)) != 0)
        goto error;
    ms = mstream_from_mfile(&mf);

    /* Version */
    if (mstream_read_u8(&ms) != VERSION)
    {
        goto error;
    }

    /* Load list of plugins. If a plugin is outdated, we don't add it to the
     * returned list and we don't load its commands */
    cached_plugin_count = mstream_read_li16(&ms);
    if (plugin_ids_resize(&cached_plugin_map, cached_plugin_count) != 0)
        goto error;
    for (cached_plugin_id = 0; cached_plugin_id != cached_plugin_count;
         ++cached_plugin_id)
    {
        int                       i;
        const struct plugin_info* plugin;

        uint64_t       cached_stamp = mstream_read_lu64(&ms);
        struct ospathc cached_path = mstream_read_ospath(&ms);

        uint64_t stamp = fs_mtime_ms(cached_path);
        if (stamp != cached_stamp)
            continue;

        vec_enumerate(plugins, i, plugin)
        {
            if (ospathc_equal(ospathc(plugin->filepath), cached_path))
            {
                plugin_ids_push(cached_plugins, i);
                cached_plugin_map->data[cached_plugin_id] = i;
                break;
            }
        }
    }

    /* Command list */
    cached_cmd_count = mstream_read_li32(&ms);
    for (cached_cmd = 0; cached_cmd != cached_cmd_count; ++cached_cmd)
    {
        int              i;
        cmd_id           cmd;
        const plugin_id* cached_plugin_id;
        struct utf8_view db_cmd_name = mstream_read_utf8(&ms);
        struct utf8_view c_symbol = mstream_read_utf8(&ms);
        plugin_id        plugin_id = mstream_read_li16(&ms);
        enum type        return_type = mstream_read_u8(&ms);
        int              param_count = mstream_read_u8(&ms);

        vec_for_each(cached_plugin_map, cached_plugin_id)
        {
            if (*cached_plugin_id == plugin_id)
                goto load_command_from_cache;
        }
        for (i = 0; i != param_count; ++i)
        {
            mstream_read_u8(&ms);
            mstream_read_utf8(&ms);
        }
        continue;

    load_command_from_cache:
        cmd = cmd_list_insert(
            cmds,
            cached_cmd,
            cached_plugin_map->data[plugin_id],
            return_type,
            db_cmd_name,
            c_symbol);
        if (cmd < 0)
            goto error;

        for (i = 0; i != param_count; ++i)
        {
            uint8_t                  data = mstream_read_u8(&ms);
            struct utf8_view         param_name = mstream_read_utf8(&ms);
            enum cmd_param_direction direction
                = (data & 0x80) ? CMD_PARAM_OUT : CMD_PARAM_IN;
            enum type param_type = (data & 0x7F);

            if (cmd_add_param(cmds, cmd, param_type, direction, param_name)
                != 0)
                goto error;
        }
    }

    return 0;

error:
    ospath_deinit(path);
    utf8_deinit(fname);
    return -1;
}

int
cmd_cache_save(
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    enum sdk_type             sdk_type,
    enum target_arch          arch,
    enum target_platform      platform)
{
    cmd_id                    cmd;
    struct mfile              mf;
    const struct plugin_info* plugin;
    struct utf8               fname = empty_utf8();
    struct ospath             path = empty_ospath();
    struct mstream            ms = mstream_init_writable();

    if (fs_get_appdata_dir(&path) != 0)
        goto error;

    if (utf8_fmt(
            &fname,
            "%s-%s-%s.dat",
            sdk_type_to_name(sdk_type),
            target_arch_to_name(arch),
            target_platform_to_name(platform))
        != 0)
    {
        goto error;
    }

    if (ospath_join_cstr(&path, "cmd-cache") != 0)
        goto error;
    if (fs_make_path(path) != 0)
        goto error;
    if (ospath_join(&path, utf8_ospathc(fname)) != 0)
        goto error;

    /* Version */
    mstream_write_u8(&ms, VERSION);

    /* List of plugins */
    mstream_write_li16(&ms, plugins->count);
    vec_for_each(plugins, plugin)
    {
        /* Timestamps of plugins, so next time we know if the plugin has to be
         * parsed again or not */
        uint64_t stamp = fs_mtime_ms(ospathc(plugin->filepath));

        mstream_write_lu64(&ms, stamp);
        mstream_write_ospath(&ms, plugin->filepath);
    }

    /* Command list */
    mstream_write_li32(&ms, cmd_list_count(cmds));
    for (cmd = 0; cmd != cmd_list_count(cmds); ++cmd)
    {
        int i;
        mstream_write_utf8(&ms, utf8_list_view(cmds->db_cmd_names, cmd));
        mstream_write_utf8(&ms, utf8_list_view(cmds->c_symbols, cmd));
        mstream_write_li16(&ms, cmds->plugin_ids->data[cmd]);
        mstream_write_u8(&ms, cmds->return_types->data[cmd]);
        mstream_write_u8(&ms, cmds->param_types->data[cmd]->count);
        for (i = 0; i != cmds->param_types->data[cmd]->count; i++)
        {
            const struct cmd_param* param_type
                = &cmds->param_types->data[cmd]->data[i];
            struct utf8_view param_name
                = utf8_list_view(cmds->db_param_names->data[cmd], i);
            mstream_write_u8(
                &ms, (param_type->type & 0x7F) | (param_type->direction << 7));
            mstream_write_utf8(&ms, param_name);
        }
    }

    /* If at any point a write failed, the error flag is set */
    if (ms.error)
        goto error;

    /* Write to file */
    if (mfile_map_overwrite(&mf, ms.ptr, ospathc(path)) != 0)
        goto error;
    memcpy(mf.address, ms.data, ms.ptr);

    mfile_unmap(&mf);
    mstream_free_writable(&ms);
    ospath_deinit(path);
    utf8_deinit(fname);
    return 0;

error:
    mstream_free_writable(&ms);
    ospath_deinit(path);
    utf8_deinit(fname);
    return -1;
}
