#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "odb-sdk/Reference.hpp"

namespace odb {
class DynamicLibData : public RefCounted {
public:
    ~DynamicLibData();

    /// @brief Opens a dynamic lib (either dll, so, or dylib) for extracting data.
    static Reference<DynamicLibData> open(const std::string& filename);

    const char* getFilename() const;

    /*!
     * @brief Returns the total number of symbols present in the symbol table.
     */
    int getSymbolCount() const;

    /*!
     * @brief Returns a symbol at the specified index in the symbol table.
     */
    std::string getSymbolNameAt(int idx) const;

    /*!
     * @brief Return the value of a null-terminated string pointed at by a symbol called 'name'.
     */
     std::optional<std::string> lookupStringBySymbol(const std::string& name);

    /*!
     * @brief Returns a copy of all strings in the string table.
     */
    std::vector<std::string> getStringTable() const;

private:
    struct Storage;

    DynamicLibData(std::unique_ptr<Storage> data, const std::string& filename);

    std::unique_ptr<Storage> data_;
    const std::string filename_;
};
}
