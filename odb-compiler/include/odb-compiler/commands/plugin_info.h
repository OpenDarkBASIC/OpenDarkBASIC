#pragma once

#include "odb-compiler/config.h"
#include "odb-sdk/ospath.h"
#include "odb-sdk/ospath_list.h"
#include "odb-sdk/utf8.h"
#include "odb-sdk/vec.h"

struct plugin_info
{
    /*!
     * @brief Absolute path to the shared library or DLL.
     */
    struct utf8 filepath;

    /*!
     * @brief The filename component without its extension. For example, if we
     * load libs/DBProCore.dll, then this will contain "DBProCore".
     */
    struct utf8 name;
};

VEC_DECLARE_API(plugin_list, struct plugin_info, 16)

ODBCOMPILER_PUBLIC_API int
plugin_list_populate(
    struct plugin_list* plugins,
    struct ospath_view  sdk_root,
    const struct ospath_list*  extra_plugin_dirs);
