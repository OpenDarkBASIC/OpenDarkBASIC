#include "odb-compiler/parsers/PluginInfo.hpp"
#include "odb-sdk/Log.hpp"

#include <LIEF/LIEF.hpp>
#include <codecvt>
#include <utility>
#include <filesystem>

#ifdef _WIN32
#undef ERROR
#endif

namespace odb {
PluginInfo::~PluginInfo() = default;

Reference<PluginInfo> PluginInfo::open(const std::string& path)
{
    try
    {
        return new PluginInfo(LIEF::Parser::parse(path), path);
    }
    catch (const LIEF::exception& err)
    {
        Log::codegen(Log::ERROR, "Failed to open plugin %s: %s", path.c_str(), err.what());
        return nullptr;
    }
}

const char *PluginInfo::getPath() const {
    return path_.c_str();
}

const char* PluginInfo::getName() const
{
    return name_.c_str();
}

size_t PluginInfo::getSymbolCount() const
{
    const auto* elfBinary = dynamic_cast<const LIEF::ELF::Binary*>(binary_.get());
    return elfBinary ? elfBinary->dynamic_symbols().size() : binary_->symbols().size();
}

std::string PluginInfo::getSymbolNameAt(size_t idx) const
{
    const auto* elfBinary = dynamic_cast<const LIEF::ELF::Binary*>(binary_.get());
    return elfBinary ? elfBinary->dynamic_symbols()[idx].name() : binary_->symbols()[idx].name();
}

std::optional<std::string> PluginInfo::lookupStringBySymbol(const std::string& name)
{
    if (!binary_->has_symbol(name)) {
        return std::nullopt;
    }

    const auto& symbol = binary_->get_symbol(name);

    // The symbol stores a virtual address to the actual string data, look that up.
    auto buffer = binary_->get_content_from_virtual_address(symbol.value(), symbol.size());
    std::uint64_t address = 0;
    std::memcpy(&address, buffer.data(), buffer.size());

    // Look up the string itself. We need to read the string in a buffered loop until we hit a null terminator.
    std::string result;
    bool foundNullTerminator = false;
    while (!foundNullTerminator)
    {
        // Load bytes from the virtual address.
        const std::uint64_t bufferSize = 64;
        auto bytes = binary_->get_content_from_virtual_address(address, bufferSize);
        address += bufferSize;

        // Have we encountered a null terminator yet?
        int size = 1;
        for (; size <= int(bytes.size()); ++size) {
            if (bytes[size] == '\0') {
                foundNullTerminator = true;
                break;
            }
        }

        // Copy 'size' bytes into the string.
        result.append(reinterpret_cast<const char*>(bytes.data()), size);

        // If 'get_content_from_virtual_address' truncated the result (because we ran out of bytes to read), then stop.
        if (bytes.size() < bufferSize) {
            break;
        }
    }

    return {result};
}

std::vector<std::string> PluginInfo::getStringTable() const
{
    const auto* binary = binary_.get();
    if (binary->format() != LIEF::FORMAT_PE) {
        // Other executable formats don't support string tables.
        return {};
    }

    const auto* peBinary = static_cast<const LIEF::PE::Binary*>(binary);

    std::vector<std::string> stringTable;
    if (peBinary->resources_manager().has_string_table())
    {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
        auto peStringTable = peBinary->resources_manager().string_table();
        stringTable.reserve(peStringTable.size());
        for (const auto& entry : peStringTable)
        {
            stringTable.push_back(conv.to_bytes(entry.name()));
        }
    }
    return stringTable;
}

PluginInfo::PluginInfo(std::unique_ptr<LIEF::Binary> binary, const std::string& path)
    : binary_(std::move(binary)),
      path_(path),
      name_(std::filesystem::path{path}.stem().string())
{
}
} // namespace odb