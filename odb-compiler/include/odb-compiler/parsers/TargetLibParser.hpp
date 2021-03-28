#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "odb-sdk/Reference.hpp"

namespace odb {
struct TargetLibParserData;

class TargetLibParser : public RefCounted {
public:
    ~TargetLibParser();

    static Reference<TargetLibParser> open(const std::string& filename);

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

    /*!
     * @brief Iterates over every string in the string table.
     */
    void forEachStringTableEntry(std::function<void(std::string)> iterator) const;

private:
    TargetLibParser(std::unique_ptr<TargetLibParserData> data, const std::string& filename);

    std::unique_ptr<TargetLibParserData> data_;
    const std::string filename_;
};
}
