#include "odb-cli/SDK.hpp"

extern "C" {
#include "odb-sdk/fs.h"
#include "odb-sdk/log.h"
#include "odb-sdk/vec.h"
}

static struct G
{
    ~G()
    {
        struct ospath* path;
        vec_for_each(plugin_dirs, path)
            ospath_free(path);
        vec_free(plugin_dirs);
    }

    struct ospath sdk_root_dir = ospath();
    VEC(struct ospath, 16)* plugin_dirs = NULL;
    odb::SDKType sdk_type = odb::SDKType::ODB;
} g;

// ----------------------------------------------------------------------------
bool setSDKRootDir(const std::vector<std::string>& args)
{
    if (g.sdk_root_dir.str.len > 0)
    {
        log_sdk_err("SDK root directory is already set to {quote:%.*s}'\n", g.sdk_root_dir.str.len, g.sdk_root_dir.str.data);
        return false;
    }

    ospath_set(&g.sdk_root_dir, cstr_utf8_view(args[0].c_str()));
    log_sdk_info("Using SDK root directory {quote:%.*s}\n", g.sdk_root_dir.str.len, g.sdk_root_dir.str.data);

    return true;
}

// ----------------------------------------------------------------------------
bool setSDKType(const std::vector<std::string>& args)
{
    if (args[0] == "dbpro") {
      g.sdk_type = odb::SDKType::DarkBASIC;
    } else if (args[0] == "odb-sdk") {
      g.sdk_type = odb::SDKType::ODB;
    } else {
      log_sdk_err("Unknown SDK type {quote:%s}\n", args[0].c_str());
      return false;
    }

    log_sdk_info("Setting SDK type to {quote:%s}\n", args[0].c_str());
    return true;
}

// ----------------------------------------------------------------------------
bool setAdditionalPluginsDir(const std::vector<std::string>& args)
{
    for (const auto& dirOrFile : args)
        ospath_set(vec_emplace(g.plugin_dirs), cstr_utf8_view(dirOrFile.c_str()));

    return true;
}

// ----------------------------------------------------------------------------
bool printSDKRootDir(const std::vector<std::string>& args)
{
    log_sdk_note("SDK root directory: {quote:%.*s}'\n", g.sdk_root_dir.str.len, g.sdk_root_dir.str.data);
    return true;
}

// ----------------------------------------------------------------------------
bool initSDK(const std::vector<std::string>& args)
{
    // Default SDK root dir
    if (g.sdk_root_dir.str.len == 0)
    {
        switch (g.sdk_type)
        {
            case odb::SDKType::ODB : {
                // Should in the same directory as the odbc executable
                struct ospath path = ospath();
                //fs_get_path_to_self(&path);
                //ospath_
                //sdkRootDir_ = FileSystem::getPathToSelf().replace_filename("odb-sdk");
            } break;

            case odb::SDKType::DarkBASIC: {
                log_sdk_err(
                    "There is no default path configured for the DarkBASIC Pro SDK "
                    "root directory. Please specify it with {emph:--sdkroot} or set "
                    "the SDK type to {quote:odb} with {emph:--sdktype}\n");
                return false;
            }
        }
    }

    log_sdk_info("Initialized SDK\n");
    return true;
}

// ----------------------------------------------------------------------------
odb::SDKType getSDKType()
{
    return g.sdk_type;
}

// ----------------------------------------------------------------------------
const std::filesystem::path getSDKRootDir()
{
    return std::string(g.sdk_root_dir.str.data, g.sdk_root_dir.str.len);
}

// ----------------------------------------------------------------------------
/*
const std::vector<std::filesystem::path>& getAdditionalPluginDirs()
{
    return pluginDirs_;
}
*/