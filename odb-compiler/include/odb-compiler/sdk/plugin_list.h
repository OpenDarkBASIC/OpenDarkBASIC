#pragma once

#include "odb-compiler/codegen/target.h"
#include "odb-compiler/config.h"
#include "odb-compiler/sdk/sdk_type.h"
#include "odb-util/ospath_list.h"
#include "odb-util/utf8.h"
#include "odb-util/vec.h"

typedef int16_t plugin_id;

struct plugin_info
{
    /*!
     * @brief Path to the shared library or DLL, relative to the SDK root.
     */
    struct ospath filepath;

    /*!
     * @brief The filename component without its extension. For example, if we
     * load libs/DBProCore.dll, then this will contain "DBProCore".
     */
    struct utf8 name;
};

VEC_DECLARE_API(ODBCOMPILER_PUBLIC_API, plugin_list, struct plugin_info, 16)

static inline void
plugin_info_init(struct plugin_info* plugin)
{
    plugin->filepath = empty_ospath();
    plugin->name = empty_utf8();
}

static inline void
plugin_info_deinit(struct plugin_info* plugin)
{
    utf8_deinit(plugin->name);
    ospath_deinit(plugin->filepath);
}

#if defined(ODBUTIL_MEM_DEBUGGING)
ODBCOMPILER_PUBLIC_API void
mem_acquire_plugin_list(struct plugin_list* plugins);
ODBCOMPILER_PUBLIC_API void
mem_release_plugin_list(struct plugin_list* plugins);
#else
#define mem_acquire_plugin_list(plugins)
#define mem_release_plugin_list(plugins)
#endif

ODBCOMPILER_PUBLIC_API plugin_id
plugin_list_add_or_get(struct plugin_list** plugins, struct ospathc filepath);

ODBCOMPILER_PUBLIC_API int
plugin_list_populate(
    struct plugin_list** plugins,
    enum sdk_type        sdk_type,
    enum target_platform target_platform,
    struct ospathc       sdk_root,
    struct ospath_list*  extra_plugins);
