#include "odb-compiler/parsers/TargetLibParser.hpp"
#include "odb-sdk/Log.hpp"

#include <LIEF/LIEF.hpp>
#include <codecvt>

namespace odb {
struct TargetLibParserData
{
    std::unique_ptr<LIEF::Binary> binary;
};

TargetLibParser::~TargetLibParser() = default;

Reference<TargetLibParser> TargetLibParser::open(const std::string& filename)
{
    try
    {
        return new TargetLibParser(
            std::make_unique<TargetLibParserData>(TargetLibParserData{LIEF::Parser::parse(filename)}), filename);
    }
    catch (const LIEF::exception& err)
    {
        Log::codegen(Log::ERROR, "Failed to load plugin %s: %s", filename.c_str(), err.what());
        return nullptr;
    }
}

const char* TargetLibParser::getFilename() const
{
    return filename_.c_str();
}

int TargetLibParser::getSymbolCount() const
{
    return 0;
}

const char* TargetLibParser::getSymbolAt(int idx) const
{
    return "";
}

std::vector<std::string> TargetLibParser::getStringTable() const
{
    const auto* binary = data_->binary.get();
    if (binary->format() != LIEF::FORMAT_PE) {
        // Other executable formats don't support string tables.
        return {};
    }

    const auto* peBinary = static_cast<const LIEF::PE::Binary*>(binary);

    std::vector<std::string> stringTable;
    if (peBinary->resources_manager().has_string_table()) {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
        auto peStringTable = peBinary->resources_manager().string_table();
        stringTable.reserve(peStringTable.size());
        for (const auto& entry : peStringTable)
        {
            stringTable.emplace_back(conv.to_bytes(entry.name()));
        }
    }

    return stringTable;
}

TargetLibParser::TargetLibParser(std::unique_ptr<TargetLibParserData> data, const std::string& filename) : data_(std::move(data)), filename_(filename)
{
}
} // namespace odb