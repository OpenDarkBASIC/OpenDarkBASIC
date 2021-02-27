#pragma once

#include <vector>
#include <string>
#include <memory>

#include "odb-sdk/Reference.hpp"

namespace odb {
struct TargetLibParserData;

class TargetLibParser : public RefCounted {
public:
    ~TargetLibParser();

    static Reference<TargetLibParser> open(const std::string& filename);

    virtual const char* getFilename() const;

    /*!
     * @brief Returns the total number of symbols present in the symbol table.
     */
    virtual int getSymbolCount() const;

    /*!
     * @brief Returns a symbol at the specified index in the symbol table.
     */
    virtual const char* getSymbolAt(int idx) const;

    /*!
     * @brief Returns a copy of all strings in the string table.
     */
    virtual std::vector<std::string> getStringTable() const;

private:
    TargetLibParser(std::unique_ptr<TargetLibParserData> data, const std::string& filename);

    std::unique_ptr<TargetLibParserData> data_;
    const std::string filename_;
};
}
