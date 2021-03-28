#include "odb-compiler/parsers/DynamicLibData.hpp"
#include "odb-sdk/Log.hpp"

#include <LIEF/LIEF.hpp>
#include <codecvt>

namespace odb {
struct DynamicLibData::Storage
{
    std::unique_ptr<LIEF::Binary> binary;
};

DynamicLibData::~DynamicLibData() = default;

Reference<DynamicLibData> DynamicLibData::open(const std::string& filename)
{
    try
    {
        return new DynamicLibData(
            std::make_unique<DynamicLibData::Storage>(DynamicLibData::Storage{LIEF::Parser::parse(filename)}), filename);
    }
    catch (const LIEF::exception& err)
    {
        Log::codegen(Log::ERROR, "Failed to load plugin %s: %s", filename.c_str(), err.what());
        return nullptr;
    }
}

const char* DynamicLibData::getFilename() const
{
    return filename_.c_str();
}

int DynamicLibData::getSymbolCount() const
{
    const auto* elfBinary = dynamic_cast<const LIEF::ELF::Binary*>(data_->binary.get());
    return elfBinary ? elfBinary->dynamic_symbols().size() : data_->binary->symbols().size();
}

std::string DynamicLibData::getSymbolNameAt(int idx) const
{
    const auto* elfBinary = dynamic_cast<const LIEF::ELF::Binary*>(data_->binary.get());
    return elfBinary ? elfBinary->dynamic_symbols()[idx].name() : data_->binary->symbols()[idx].name();
}

std::optional<std::string> DynamicLibData::lookupStringBySymbol(const std::string& name)
{
    if (!data_->binary->has_symbol(name)) {
        return std::nullopt;
    }

    const auto& symbol = data_->binary->get_symbol(name);

    // The symbol stores a virtual address to the actual string data, look that up.
    auto buffer = data_->binary->get_content_from_virtual_address(symbol.value(), symbol.size());
    std::uint64_t address = 0;
    std::memcpy(&address, buffer.data(), buffer.size());

    // Look up the string itself. We need to read the string in a buffered loop until we hit a null terminator.
    std::string result;
    bool foundNullTerminator = false;
    while (!foundNullTerminator)
    {
        // Load bytes from the virtual address.
        const std::uint64_t bufferSize = 64;
        auto bytes = data_->binary->get_content_from_virtual_address(address, bufferSize);
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

std::vector<std::string> DynamicLibData::getStringTable() const
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
            stringTable.push_back(conv.to_bytes(entry.name()));
        }
    }
    return stringTable;
}

DynamicLibData::DynamicLibData(std::unique_ptr<Storage> data, const std::string& filename) : data_(std::move(data)), filename_(filename)
{
}
} // namespace odb