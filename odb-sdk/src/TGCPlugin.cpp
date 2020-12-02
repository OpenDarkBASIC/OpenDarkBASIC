#include "odb-sdk/TGCPlugin.hpp"
#include <iostream>
#include <cassert>
#include <algorithm>
#include <filesystem>

#ifdef ODBSDK_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace odbc {

namespace {

template <class Container>
void split(const std::string &str, Container &cont,
           char delim = ' ')
{
    std::size_t current, previous = 0;
    current = str.find(delim);
    while (current != std::string::npos)
    {
        cont.push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find(delim, previous);
    }
    cont.push_back(str.substr(previous, current - previous));
}

#ifdef ODBSDK_PLATFORM_WIN32
std::vector<std::string> extractStringTableFromPEFile(HMODULE hModule)
{
    std::vector<std::string> stringTableEntries;
    char buffer[512];
    int resourceIdx = 1;
    while (true)
    {
        int charactersRead = LoadStringA(hModule, resourceIdx++, buffer, 512);
        if (charactersRead == 0)
        {
            break;
        }
        stringTableEntries.emplace_back(buffer, charactersRead);
    }
    return stringTableEntries;
}
#endif

}

// ----------------------------------------------------------------------------
std::unique_ptr<TGCPlugin> TGCPlugin::open(const char* filename)
{
#ifdef ODBSDK_PLATFORM_WIN32
    HMODULE hModule = LoadLibraryExA(filename, nullptr, LOAD_LIBRARY_AS_DATAFILE);
    if (!hModule)
    {
        auto error = GetLastError();
        std::cerr << "Failed to load library: " << error << std::endl;
        return nullptr;
    }

    return std::unique_ptr<TGCPlugin>(new TGCPlugin(static_cast<void*>(hModule), std::filesystem::path{filename}.filename().string()));
#else
    return nullptr;
#endif
}

// ----------------------------------------------------------------------------
TGCPlugin::~TGCPlugin()
{
#ifdef ODBSDK_PLATFORM_WIN32
    FreeLibrary(static_cast<HMODULE>(handle_));
#endif
}

// ----------------------------------------------------------------------------
bool TGCPlugin::loadKeywords(KeywordDB* db) const
{
#ifdef ODBSDK_PLATFORM_WIN32
    std::vector<std::string> stringTableEntries = extractStringTableFromPEFile(static_cast<HMODULE>(handle_));
    if (stringTableEntries.empty())
    {
        return false;
    }

    // Populate keyword database.
    for (const auto& entry : stringTableEntries)
    {
        std::vector<std::string> tokens;
        split(entry, tokens, '%');

        if (tokens.size() < 2) {
            fprintf(stderr, "Invalid string table entry: %s\n", entry.c_str());
            continue;
        }

        auto convertTypeChar = [](char type) -> Keyword::Type
        {
            return static_cast<Keyword::Type>(type);
        };

        // Extract keyword name and return type.
        auto& keywordName = tokens[0];
        auto& functionTypes = tokens[1];
        const auto& dllSymbol = tokens[2];
        std::optional<Keyword::Type> returnType;

        // Extract return type.
        if (keywordName.back() == '[')
        {
            keywordName = keywordName.substr(0, keywordName.size() - 1);
            returnType = convertTypeChar(tokens[1][0]);
            functionTypes = functionTypes.substr(1);
        }
        std::transform(keywordName.begin(), keywordName.end(), keywordName.begin(), [](char c) { return std::tolower(c); });

        // Create overload.
        Keyword::Overload overload;
        overload.symbolName = dllSymbol;
        overload.returnType = returnType;

        std::vector<std::string> argumentNames;
        if (tokens.size() > 3)
        {
            split(tokens[3], argumentNames, ',');
        }
        for (int i = 0; i < functionTypes.size(); ++i)
        {
            Keyword::Arg arg;
            arg.type = convertTypeChar(functionTypes[i]);
            if (arg.type == Keyword::Type::Void) {
                continue;
            }
            if (i < argumentNames.size()) {
                arg.description = std::move(argumentNames[i]);
            }
            overload.args.emplace_back(std::move(arg));
        }

        // Add to database, or merge with existing keyword if it exists already.
        Keyword* existingKeyword = db->lookup(keywordName);
        if (existingKeyword)
        {
            existingKeyword->overloads.emplace_back(std::move(overload));
        }
        else
        {
            Keyword kw;
            kw.name = std::move(keywordName);
            kw.overloads.emplace_back(std::move(overload));
            kw.plugin = pluginName_;
            db->addKeyword(kw);
        }
    }
    return true;
#else
    return false;
#endif
}

}
