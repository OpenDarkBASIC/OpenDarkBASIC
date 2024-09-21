#include "odb-cli/Codegen.hpp"
#include "odb-cli/SDK.hpp"

extern "C" {
#include "odb-compiler/sdk/sdk_type.h"
#include "odb-util/fs.h"
#include "odb-util/log.h"
#include "odb-util/ospath.h"
#include "odb-util/ospath_list.h"
}

static struct ospath       sdk_root_dir;
static enum sdk_type       sdk_type;
static struct ospath_list* plugin_dirs;

void
initSDK(void)
{
    sdk_type = SDK_ODB;
    sdk_root_dir = empty_ospath();
    ospath_list_init(&plugin_dirs);
}

void
deinitSDK(void)
{
    ospath_list_deinit(plugin_dirs);
    ospath_deinit(sdk_root_dir);
}

// ----------------------------------------------------------------------------
bool
setSDKRootDir(const std::vector<std::string>& args)
{
    if (ospath_len(sdk_root_dir) > 0)
    {
        log_err(
            "[sdk] ",
            "SDK root directory is already set to {quote:%s}'\n",
            ospath_cstr(sdk_root_dir));
        return false;
    }

    ospath_set_cstr(&sdk_root_dir, args[0].c_str());

    return true;
}

// ----------------------------------------------------------------------------
bool
setSDKType(const std::vector<std::string>& args)
{
    if (args[0] == "dbpro")
    {
        sdk_type = SDK_DBPRO;
    }
    else if (args[0] == "odb-sdk")
    {
        sdk_type = SDK_ODB;
    }
    else
    {
        log_err("[sdk] ", "Unknown SDK type {quote:%s}\n", args[0].c_str());
        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
bool
setAdditionalPluginsDir(const std::vector<std::string>& args)
{
    for (const auto& dirOrFile : args)
        ospath_list_add_cstr(&plugin_dirs, dirOrFile.c_str());

    return true;
}

// ----------------------------------------------------------------------------
bool
printSDKRootDir(const std::vector<std::string>& args)
{
    log_note("[sdk] ",
        "SDK root directory: {quote:%s}\n", ospath_cstr(sdk_root_dir));
    return true;
}

// ----------------------------------------------------------------------------
bool
setupSDK(const std::vector<std::string>& args)
{
    // Default SDK root dir
    if (ospath_len(sdk_root_dir) == 0)
    {
        switch (sdk_type)
        {
            case SDK_ODB: {
                // <arch>/<platform>/bin/odb-cli
                if (fs_get_path_to_self(&sdk_root_dir) != 0)
                    return false;
                ospath_dirname(&sdk_root_dir);
                ospath_dirname(&sdk_root_dir);
                ospath_dirname(&sdk_root_dir);
                ospath_dirname(&sdk_root_dir);
                if (ospath_join_cstr(&sdk_root_dir, target_arch_to_name(getTargetArch())) != 0)
                    return false;
                if (ospath_join_cstr(&sdk_root_dir, target_platform_to_name(getTargetPlatform())) != 0)
                    return false;
                if (ospath_join_cstr(&sdk_root_dir, "odb-sdk") != 0)
                    return false;
            }
            break;

            case SDK_DBPRO: {
                log_err("[sdk] ",
                    "There is no default path configured for the DarkBASIC Pro "
                    "SDK "
                    "root directory. Please specify it with {emph:--sdk-root} "
                    "or "
                    "set "
                    "the SDK type to {quote:odb} with {emph:--sdk-type odb}\n");
                return false;
            }
        }
    }

    if (sdk_type == SDK_DBPRO)
        setPlatform({"windows"});

    /* Print to log */
    const char* type = "";
    switch (sdk_type)
    {
        case SDK_ODB: type = "OpenDarkBASIC"; break;
        case SDK_DBPRO: type = "DarkBASIC Pro"; break;
    }
    log_info("[sdk] ",
        "Using {emph:%s SDK} with root directory {quote:%s}\n",
        type,
        ospath_cstr(sdk_root_dir));

    return true;
}

// ----------------------------------------------------------------------------
enum sdk_type
getSDKType()
{
    return sdk_type;
}

// ----------------------------------------------------------------------------
struct ospathc
getSDKRootDir()
{
    return ospathc(sdk_root_dir);
}

// ----------------------------------------------------------------------------
struct ospath_list* getSDKPluginDirs()
{
    return plugin_dirs;
}
