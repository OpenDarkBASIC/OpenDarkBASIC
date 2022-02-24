#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "odb-sdk/Reference.hpp"

namespace LIEF {
class Binary;
}

namespace odb {
class PluginInfo : public RefCounted {
public:
    ~PluginInfo();

    /// @brief Opens a dynamic lib (either dll, so, or dylib) for extracting data.
    static Reference<PluginInfo> open(const std::string& path);

    /*!
     * @brief Returns the path of the dynamic lib.
     */
    const char* getPath() const;

    /*!
     * @brief Returns the filename of the dynamic lib without it's extension. For example, if we open
     * libs/DBProCore.dll, then this function will return 'DBProCore".
     */
    const char* getName() const;

    /*!
     * @brief Returns the total number of symbols present in the symbol table.
     */
    size_t getSymbolCount() const;

    /*!
     * @brief Returns a symbol at the specified index in the symbol table.
     */
    std::string getSymbolNameAt(size_t idx) const;

    /*!
     * @brief Return the value of a null-terminated string pointed at by a symbol called 'name'.
     */
     std::optional<std::string> lookupStringBySymbol(const std::string& name);

    /*!
     * @brief Returns a copy of all strings in the string table.
     */
    std::vector<std::string> getStringTable() const;

private:
    PluginInfo(std::unique_ptr<LIEF::Binary> binary, const std::string& path);

    std::unique_ptr<LIEF::Binary> binary_;
    const std::string path_;
    const std::string name_;
};
}
