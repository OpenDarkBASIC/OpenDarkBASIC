#include "odbc/parsers/keywords/KeywordDLL.hpp"
#include <iostream>
#include <cassert>
#include <algorithm>

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace odbc {
namespace {
template <class Container>
void split(const std::string &str, Container &cont,
           char delim = ' ') {
    std::size_t current, previous = 0;
    current = str.find(delim);
    while (current != std::string::npos) {
        cont.push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find(delim, previous);
    }
    cont.push_back(str.substr(previous, current - previous));
}
}

bool addKeywordDBFromDLL(KeywordDB& keywordDB, const std::string& dllPath) {
#ifdef _MSC_VER
    std::vector<std::string> stringTableEntries;
    fprintf(stderr, "Loading plugin %s\n", dllPath.c_str());

    // Read string resources from library.
    HMODULE hModule = ::LoadLibraryExA(dllPath.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);
    if (hModule == nullptr) {
        auto error = GetLastError();
        std::cerr << "Failed to load library: " << error << std::endl;
        return false;
    }
    char buffer[512];
    int resourceIdx = 1;
    while (true) {
        int charactersRead = LoadStringA(hModule, resourceIdx++, buffer, 512);
        if (charactersRead == 0) {
            break;
        }
        stringTableEntries.emplace_back(buffer, charactersRead);
    }
    FreeLibrary(hModule);

    // Populate keyword database.
    for (const auto& entry : stringTableEntries) {
        std::vector<std::string> tokens;
        split(entry, tokens, '%');

        if (tokens.size() < 2) {
            fprintf(stderr, "Invalid string table entry: %s\n", entry.c_str());
            continue;
        }

        auto convertTypeChar = [](char type) -> Keyword::Type {
            return static_cast<Keyword::Type>(type);
        };

        // Extract keyword name and return type.
        auto& keywordName = tokens[0];
        auto& functionTypes = tokens[1];
        const auto& dllSymbol = tokens[2];
        std::optional<Keyword::Type> returnType;

        // Extract return type.
        if (keywordName.back() == '[') {
            keywordName = keywordName.substr(0, keywordName.size() - 1);
            returnType = convertTypeChar(tokens[1][0]);
            functionTypes = functionTypes.substr(1);
        }
        std::transform(keywordName.begin(), keywordName.end(), keywordName.begin(), [](char c) { return std::tolower(c); });

        // Create overload.
        Keyword::Overload overload;
        overload.dllSymbol = dllSymbol;

        std::vector<std::string> argumentNames;
        if (tokens.size() > 3) {
            split(tokens[3], argumentNames, ',');
        }
        for (int i = 0; i < functionTypes.size(); ++i) {
            Keyword::Arg arg;
            arg.type = convertTypeChar(functionTypes[i]);
            if (i < argumentNames.size()) {
                arg.description = std::move(argumentNames[i]);
            }
            overload.args.emplace_back(std::move(arg));
        }

        // Add to database, or merge with existing keyword if it exists already.
        Keyword* existingKeyword = keywordDB.lookup(keywordName);
        if (existingKeyword) {
            existingKeyword->overloads.emplace_back(std::move(overload));
        } else {
            Keyword kw;
            kw.name = std::move(keywordName);
            kw.returnType = returnType;
            kw.overloads.emplace_back(std::move(overload));
            keywordDB.addKeyword(kw);
        }
    }
    return true;
#else
    std::cerr << "Plugin loading only works on Windows." << std::endl;
    return false;
#endif
}

}