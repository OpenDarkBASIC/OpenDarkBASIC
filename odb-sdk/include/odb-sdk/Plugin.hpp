#pragma once

#include "odb-sdk/config.hpp"
#include <memory>
#include <string>

namespace odb {

struct PluginPlatformData;
class KeywordIndex;

class ODBSDK_PUBLIC_API Plugin
{
public:
    Plugin() = delete;
    Plugin(const Plugin& other) = delete;
    Plugin(Plugin&& other) = default;
    ~Plugin();

    /*!
     * @brief Attempts to load the specified shared library or DLL.
     * @arg openAsDataFile If this is set to true, then this plugin is loaded as data only, rather
     * than being loaded as executable. Symbol addresses from data only plugins will raise a
     * read-only access violation when called.
     */
    static std::unique_ptr<Plugin> open(const char* filename, bool openAsDataFile = false);

    /*!
     * @brief The name of the plugin.
     */
    std::string getName() const;

    /*!
     * @brief Looks up the address of a given symbol in the shared library or
     * DLL. This is equivalent to calling dlsym() on linux and GetProcAddress()
     * on Windows.
     */
    void* lookupSymbolAddress(const char* name) const;

    /*!
     * @brief Returns the total number of symbols present in the symbol table.
     */
    int getSymbolCount() const;

    /*!
     * @brief Returns a symbol at the specified index in the symbol table.
     */
    const char* getSymbolAt(int idx) const;

    /*!
     * @brief Returns the total number of strings present in the string table.
     */
    int getStringTableSize() const;

    /*!
     * @brief Returns a string at the specified index in the string table.
     */
    std::string getStringTableEntryAt(int idx) const;

private:
    explicit Plugin(std::unique_ptr<PluginPlatformData> data);
    std::unique_ptr<PluginPlatformData> data_;
};

}
