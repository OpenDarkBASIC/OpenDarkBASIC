#pragma once

#include <memory>
#include "odb-sdk/runtime/config.hpp"

namespace odb {

struct PluginPlatformData;
class KeywordIndex;

class Plugin
{
public:
    Plugin() = delete;
    Plugin(const Plugin& other) = delete;
    Plugin(Plugin&& other) = default;
    ~Plugin();

    /*!
     * @brief Attempts to load the specified shared library or DLL.
     */
    static std::unique_ptr<Plugin> open(const char* filename);

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

private:
    Plugin(std::unique_ptr<PluginPlatformData> data);
    std::unique_ptr<PluginPlatformData> data_;
};

}
