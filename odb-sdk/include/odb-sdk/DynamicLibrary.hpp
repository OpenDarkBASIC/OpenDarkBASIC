#pragma once

#include "odb-sdk/config.hpp"
#include "odb-sdk/RefCounted.hpp"
#include <memory>
#include <string>
#include <vector>

namespace odb {

struct DynLibPlatformData;

class ODBSDK_PUBLIC_API DynamicLibrary : public RefCounted
{
public:
    DynamicLibrary() = delete;
    ~DynamicLibrary();

    /*!
     * @brief Attempts to load the specified shared library or DLL.
     * @return Returns nullptr on failure, otherwise returns a new instance of
     * this class.
     */
    static DynamicLibrary* open(const char* filename);

    const char* getFilename() const;

    /*!
     * @brief Looks up the address of a given symbol in the shared library or
     * DLL. This is equivalent to calling dlsym() on linux and GetProcAddress()
     * on Windows.
     */
    void* lookupSymbolAddress(const char* name) const;

    /*!
     * @brief Returns the total number of symbols present in the symbol table.
     */
    int getSymbolCount() const;

    /*!
     * @brief Returns a symbol at the specified index in the symbol table.
     */
    const char* getSymbolAt(int idx) const;

private:
    explicit DynamicLibrary(std::unique_ptr<DynLibPlatformData> data, const std::string& filename);
    std::unique_ptr<DynLibPlatformData> data_;
    const std::string filename_;
};

}
