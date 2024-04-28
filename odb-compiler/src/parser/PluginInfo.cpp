#include "odb-compiler/parsers/PluginInfo.hpp"
#include "odb-sdk/Log.hpp"

//#include<LIEF / LIEF.hpp>
#include <codecvt>
#include <filesystem>
#include <utility>

#ifdef _WIN32
#undef ERROR
#endif

namespace odb {
PluginInfo::~PluginInfo() = default;

Reference<PluginInfo> PluginInfo::open(const std::string& path)
{
    /*try
    {
        return new PluginInfo(LIEF::Parser::parse(path), path);
    }
    catch (const LIEF::exception& err)
    {
#ifdef ERROR
#undef ERROR
#endif
        Log::cmd(Log::ERROR, "Failed to open plugin %s: %s", path.c_str(), err.what());
        return nullptr;
    }*/

    return nullptr;
}

const char* PluginInfo::getPath() const
{
    return path_.c_str();
}

const char* PluginInfo::getName() const
{
    return name_.c_str();
}

size_t PluginInfo::getSymbolCount() const
{
    /*const auto* elfBinary = dynamic_cast<const LIEF::ELF::Binary*>(binary_.get());
    return elfBinary ? elfBinary->dynamic_symbols().size() : binary_->symbols().size();*/
    return 0;
}

std::string PluginInfo::getSymbolNameAt(size_t idx) const
{
    /*const auto* elfBinary = dynamic_cast<const LIEF::ELF::Binary*>(binary_.get());
    return elfBinary ? elfBinary->dynamic_symbols()[idx].name() : binary_->symbols()[idx].name();*/
    return "";
}

std::optional<std::string> PluginInfo::lookupStringBySymbol(const std::string& name)
{
    if (!binary_->has_symbol(name))
    {
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
        for (; size <= int(bytes.size()); ++size)
        {
            if (bytes[size] == '\0')
            {
                foundNullTerminator = true;
                break;
            }
        }

        // Copy 'size' bytes into the string.
        result.append(reinterpret_cast<const char*>(bytes.data()), size);

        // If 'get_content_from_virtual_address' truncated the result (because we ran out of bytes to read), then stop.
        if (bytes.size() < bufferSize)
        {
            break;
        }
    }

    return {result};
}

std::vector<std::string> PluginInfo::getStringTable() const
{
    const auto* binary = binary_.get();
    if (binary->format() != LIEF::FORMAT_PE)
    {
        // Other executable formats don't support string tables.
        return {};
    }

    const auto* peBinary = static_cast<const LIEF::PE::Binary*>(binary);

    // Find the string table resource node.
    const auto& nodes = peBinary->resources().childs();
    auto stringTableNodeIt =
        std::find_if(std::begin(nodes), std::end(nodes),
                     [](const LIEF::PE::ResourceNode& node)
                     { return static_cast<LIEF::PE::RESOURCE_TYPES>(node.id()) == LIEF::PE::RESOURCE_TYPES::STRING; });
    if (stringTableNodeIt == std::end(nodes))
    {
        return {};
    }
    if (stringTableNodeIt->childs().size() == 0 || stringTableNodeIt->childs()[0].childs().size() == 0)
    {
        return {};
    }

    // Check if we have a licensed plugin by looking at the first bytes of the first string table entry.
    bool licensedPlugin = false;
    {
        const auto* stringTableNode =
            dynamic_cast<const LIEF::PE::ResourceData*>(&stringTableNodeIt->childs()[0].childs()[0]);
        if (stringTableNode)
        {
            const std::vector<uint8_t>& content = stringTableNode->content();
            const std::string header = " LICENSED PLUGIN:";
            if (content.size() > header.size())
            {
                if (std::string(reinterpret_cast<const char*>(content.data()), header.size()) == header)
                {
                    licensedPlugin = true;
                }
            }
        }
    }

    std::vector<std::string> stringTable;

    // If this is a licensed plugin, then we need to handle a custom format.
    //
    // Essentially, instead of UTF-16 encoded strings, we have a single ASCII string (split across all the string
    // tables) with null terminators separating different command entries.
    if (licensedPlugin)
    {
        // Combine all string table entries into a single buffer (as some commands will be split between string table
        // nodes).
        std::string buffer;
        for (const LIEF::PE::ResourceNode& childL1 : stringTableNodeIt->childs())
        {
            for (const LIEF::PE::ResourceNode& childL2 : childL1.childs())
            {
                const auto* stringTableNode = dynamic_cast<const LIEF::PE::ResourceData*>(&childL2);
                if (!stringTableNode)
                {
                    continue;
                }

                const std::vector<uint8_t>& content = stringTableNode->content();
                if (content.empty())
                {
                    continue;
                }

                buffer += std::string(reinterpret_cast<const char*>(content.data()), content.size());
            }
        }

        const char* dataPtr = buffer.data();
        const char* dataEnd = dataPtr + buffer.size();

        // Skip past the header.
        while (dataPtr < dataEnd && * dataPtr != '\0')
        {
            dataPtr++;
        }

        // Skip past the null terminator we've landed on.
        dataPtr++;

        // Read all null terminated strings stored in the buffer from this point.
        const char* currentStringPtr = dataPtr;
        while (dataPtr < dataEnd)
        {
            // If we encounter a null terminator, terminate the current string and start a new string.
            if (*dataPtr == '\0')
            {
                // Add the string to the string table, but only if it has more than one character (i.e. the first
                // character is not a null terminator).
                if (*currentStringPtr != '\0')
                {
                    stringTable.emplace_back(currentStringPtr);
                }
                currentStringPtr = dataPtr + 1;
            }
            dataPtr++;
        }
    }
    else // Otherwise, just treat it as a standard string table.
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

PluginInfo::PluginInfo(const std::string& path)
    : path_(path)
    , name_(std::filesystem::path{path}.stem().string())
{
}
} // namespace odb
