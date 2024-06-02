#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/sdk/sdk.h"
#include "odb-compiler/codegen/codegen.h"
#include "odb-sdk/ospath_list.h"
#include "odb-sdk/utf8.h"
#include "odb-sdk/vec.h"

typedef int16_t plugin_id;

struct plugin_info
{
    /*!
     * @brief Absolute path to the shared library or DLL.
     */
    struct ospath filepath;

    /*!
     * @brief The filename component without its extension. For example, if we
     * load libs/DBProCore.dll, then this will contain "DBProCore".
     */
    struct utf8 name;
};

VEC_DECLARE_API(plugin_list, struct plugin_info, 16, ODBCOMPILER_PUBLIC_API)

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

ODBCOMPILER_PUBLIC_API int
plugin_list_populate(
    struct plugin_list*       plugins,
    enum sdk_type             sdk_type,
    enum odb_codegen_platform target_platform,
    struct ospathc            sdk_root,
    const struct ospath_list* extra_plugins);
