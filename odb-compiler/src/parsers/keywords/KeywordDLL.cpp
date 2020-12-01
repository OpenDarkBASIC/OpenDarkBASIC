#include "odbc/parsers/keywords/KeywordDLL.hpp"
#include <iostream>
#include <cassert>
#include <algorithm>
#include <filesystem>

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <libpe/pe.h>
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

std::vector<std::string> extractStringTableFromPEFile(const std::string& filename) {
    std::vector<std::string> stringTableEntries;

#ifdef _MSC_VER
    // Read string resources from library.
    HMODULE hModule = ::LoadLibraryExA(filename.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);
    if (hModule == nullptr) {
        auto error = GetLastError();
        std::cerr << "Failed to load library: " << error << std::endl;
        return {};
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
#else
    pe_ctx_t ctx;
    pe_err_e err = pe_load_file(&ctx, plugin.c_str());
    if (err != LIBPE_E_OK) {
        pe_error_print(stderr, err);
        continue;
    }
    err = pe_parse(&ctx);
    if (err != LIBPE_E_OK) {
        pe_error_print(stderr, err);
        continue;
    }

    // Recurse resource tree to discover string tables.
    pe_resources_t *resources = pe_resources(&ctx);
    if (resources == nullptr || resources->err != LIBPE_E_OK) {
        fprintf(stderr, "This file has no resources");
        continue;
    }
    pe_resource_node_t *rootNode = resources->root_node;
    auto visitNode = [&ctx, &stringTableEntries](auto &&visitNode, pe_resource_node_t *node) {
      if (!node) {
          return;
      }

      // If we're at the resource directory level (level 1), stop traversing the tree further unless it's of type RT_STRING (string table entries).
      if (node->type == LIBPE_RDT_RESOURCE_DIRECTORY && node->dirLevel == LIBPE_RDT_LEVEL1) {
          if (node->parentNode != NULL && node->parentNode->type == LIBPE_RDT_DIRECTORY_ENTRY) {
              IMAGE_RESOURCE_DIRECTORY_ENTRY *dirEntry = node->parentNode->raw.directoryEntry;
              if (dirEntry->u0.Id != RT_STRING) {
                  return;
              }
          }
      }

      // If we've hit a data entry leaf node, we can assume that it's a string table entry (due to the check above).
      if (node->type == LIBPE_RDT_DATA_ENTRY) {
          const IMAGE_RESOURCE_DATA_ENTRY *entry = node->raw.dataEntry;

          // Read data entry.
          const uint64_t rawDataOffset = pe_rva2ofs(&ctx, entry->OffsetToData);
          const size_t rawDataSize = entry->Size;
          const void *rawDataPtr = LIBPE_PTR_ADD(ctx.map_addr, rawDataOffset);
          if (!pe_can_read(&ctx, rawDataPtr, rawDataSize)) {
              return;
          }

          // rawDataPtr is a pointer to a UTF-16 buffer. We can use libpe to convert it to an ASCII buffer.
          const size_t bufferSize = rawDataSize / 2;
          char* bufferPtr = new char[bufferSize];
          pe_utils_str_widechar2ascii(bufferPtr, reinterpret_cast<const char*>(rawDataPtr), bufferSize);

          // Parse string table.
          int i = 0;
          while (i < bufferSize) {
              // The first byte in the string table entry is the length of the string.
              std::size_t tableEntrySize = static_cast<std::uint8_t>(bufferPtr[i++]);
              assert((i + tableEntrySize) <= bufferSize);
              if (tableEntrySize == 0) {
                  continue;
              }

              // Read string table entry.
              // This uses the `std::string{const char *, size_t}` constructor in place in the unordered_map.
              stringTableEntries.emplace_back(&bufferPtr[i], tableEntrySize);
              i += tableEntrySize;
          }

          delete [] bufferPtr;
      }
      visitNode(visitNode, node->childNode);
      visitNode(visitNode, node->nextNode);
    };
    visitNode(visitNode, rootNode);
#endif

    return stringTableEntries;
}
}

bool addKeywordDBFromDLL(KeywordDB& keywordDB, const std::string& dllPath) {
    std::vector<std::string> stringTableEntries;
    fprintf(stderr, "Loading plugin %s\n", dllPath.c_str());

    stringTableEntries = extractStringTableFromPEFile(dllPath);
    if (stringTableEntries.empty()) {
        return false;
    }

    std::string pluginName = std::filesystem::path{dllPath}.filename().string();

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
        overload.symbolName = dllSymbol;
        overload.returnType = returnType;

        std::vector<std::string> argumentNames;
        if (tokens.size() > 3) {
            split(tokens[3], argumentNames, ',');
        }
        for (int i = 0; i < functionTypes.size(); ++i) {
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
        Keyword* existingKeyword = keywordDB.lookup(keywordName);
        if (existingKeyword) {
            existingKeyword->overloads.emplace_back(std::move(overload));
        } else {
            Keyword kw;
            kw.name = std::move(keywordName);
            kw.overloads.emplace_back(std::move(overload));
            kw.plugin = pluginName;
            keywordDB.addKeyword(kw);
        }
    }
    return true;
}

}