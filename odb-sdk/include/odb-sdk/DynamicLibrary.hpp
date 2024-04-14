#pragma once

#include "odb-sdk/config.hpp"
#include <functional>
#include <string>

namespace odb {

class ODBSDK_PUBLIC_API DynamicLibrary
{
private:
    static void* openLibrary(const char* filename);
    static void closeLibrary(void* handle);

    DynamicLibrary(void* handle) : handle_(handle) {}

public:
    enum IterateStatus
    {
        ERROR = -1,
        CONTINUE,
        STOP
    };

    DynamicLibrary() = delete;
    ~DynamicLibrary()
    {
        if (handle_)
            closeLibrary(handle_);
    }

    DynamicLibrary(const DynamicLibrary&) = delete;

    DynamicLibrary(DynamicLibrary&& other) noexcept
        : handle_(other.handle_)
    { 
        other.handle_ = nullptr;
    }

    //! Evaluate object to "true" if the library loaded successfully
    operator bool() const { return handle_ != nullptr; }

    /*!
     * \brief Appends a path to search for shared libraries.
     *
     * This is required for plugins that ship with their own shared library
     * dependencies. If the search path is not updated, then loading these plugins
     * will fail because the dynamic linker won't be ablve to find its dependencies.
     *
     * On Windows this calls SetDllDirectory()
     * On Linux this appends a path to LD_LIBRARY_PATH
     * \param[in] path Search path to add.
     * \return Returns 0 on success. Negative on error.
     */
    static int addRuntimePath(const char* path);

    /*!
     * \brief Attempts to load the specified shared library or DLL.
     * This class overloads the boolean operator, allowing you to check for
     * success with:
     * 
     *   if (auto lib = DynamicLibrary::open(...)) {
     *       // success
     *   }
     */
    static DynamicLibrary open(const char* filepath) { return DynamicLibrary(openLibrary(filepath)); }

    /*!
     * \brief Returns the fully qualified path to the shared library.
     * \note This may be different from the original filename specified in
     * DynamicLibrary::open().
     */
    std::string getFilePath() const;

    /*!
     * @brief Looks up the address of a given symbol in the shared library or
     * DLL. This is equivalent to calling dlsym() on linux and GetProcAddress()
     * on Windows.
     * \param[in] name Symbol name
     * \return Returns the address to the symbol if successful or NULL on error.
     */
    void* lookupSymbolAddressVP(const char* name) const;

    /*!
     * @brief Same as lookupSymbolAddressVP(), but casts the return value to
     * the required type.
     */
    template <typename T>
    T lookupSymbolAddress(const char* name) const
        { return static_cast<T>(lookupSymbolAddressVP(name)); }

    /*!
     * \brief Returns the number symbols in the library. These are strings
     * that have been exported (on Windows) or otherwise made externally 
     * visible. You can get their name using getSymbolName() and get their
     * address using lookupSymbolAddress().
     * 
     * \note On Windows this is a fast operation, on Linux (GNU) the shared
     * library's hash table needs to be iterated which is slightly slower.
     * 
     * \note On Linux, shared libraries often have a lot of public symbols,
     * because the default visibility for GCC is public.
     */
    //int findSymbolCount() const;

    /*!
     * \brief Returns the name of a symbol at a specified index.
     * \param[in] idx 0 to findSymbolCount() - 1
     */
    //const char* getSymbolName(int idx) const;

    IterateStatus forEachSymbol(std::function<IterateStatus(const char* symbol)> callback);

#if defined(_WIN32)
    /*!
     * \brief Returns the number of entries in the string table. These only
     * exist in Windows DLLs, and are typically created using resource files
     * (see: "Stringtable resources" https://learn.microsoft.com/en-us/windows/win32/menurc/stringtable-resource)
     * 
     * \note This operation is slow, because it must iterate over all strings
     * to find the total count.
     */
    //int findStringResourceCount() const;

    /*!
     * \brief Returns a string resource at a specified index.
     * \param[in] idx 0 to findStringResourceCount() - 1
     */
    //const char* getStringResource(int idx) const;

    /*!
     * \brief Iterates over each string resource present in the DLLs resource
     * section. These only exist in Windows DLLs, and are typically created
     * using resource files
     * (see: "Stringtable resources" https://learn.microsoft.com/en-us/windows/win32/menurc/stringtable-resource)
     */
    IterateStatus forEachString(std::function<IterateStatus(const char* str)> callback);
#endif

private:
    void* handle_;
};

}
