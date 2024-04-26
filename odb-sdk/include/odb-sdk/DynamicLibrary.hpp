#pragma once

#include "odb-sdk/config.h"
#include <functional>
#include <string>

namespace odb {

class ODBSDK_PUBLIC_API DynamicLibrary
{
private:
    static void* openImpl(const char* filename);
    static void closeImpl(void* handle);
    static void* lookupSymbolImpl(void* handle, const char* name);

    DynamicLibrary() = delete;
    DynamicLibrary(void* handle) : handle_(handle) {}
    DynamicLibrary(const DynamicLibrary&) = delete;

public:
    enum IterateStatus
    {
        ERROR = -1,
        CONTINUE,
        STOP
    };

    ~DynamicLibrary()
    {
        if (handle_)
            closeImpl(handle_);
    }

    DynamicLibrary(DynamicLibrary&& other) noexcept
        : handle_(other.handle_)
    { 
        other.handle_ = nullptr;
    }

    //! Evaluate object to "true" if the library loaded successfully
    operator bool() const
        { return handle_ != nullptr; }

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
     * \return Returns true on success, false on error.
     */
    static bool addRuntimePath(const char* path);

    /*!
     * \brief Attempts to load the specified shared library or DLL.
     * This class overloads the boolean operator, allowing you to check for
     * success with:
     * 
     *   if (auto lib = DynamicLibrary::open(...)) {
     *       // success
     *   }
     */
    static DynamicLibrary open(const char* filepath)
        { return DynamicLibrary(openImpl(filepath)); }

    /*!
     * \brief Returns the fully qualified path to the shared library.
     * \note This may be different from the original filename specified in
     * DynamicLibrary::open().
     */
    void getFilePath(std::function<void(const char*)> callback) const;

    /*!
     * @brief Looks up the address of a specified symbol in the shared library
     * or DLL, and casts it to a function pointer type.
     * This is equivalent to calling dlsym() on linux and GetProcAddress()
     * on Windows.
     * \param[in] name Symbol name
     * \return Returns the address to the symbol if successful or NULL on error.
     */
    template <typename Ret, typename... Args>
    Ret (*lookupFunction(const char* name) const)(Args&&... args)
        { return reinterpret_cast<Ret (*)(Args&&... args)>(lookupSymbolImpl(handle_, name)); }
    
    /*!
     * @brief Looks up the address of a specified symbol in the shared library
     * or DLL, and casts it to a type.
     * This is equivalent to calling dlsym() on linux and GetProcAddress()
     * on Windows.
     * \param[in] name Symbol name
     * \return Returns the address to the symbol if successful or NULL on error.
     */
    template <typename T>
    T lookupSymbol(const char* name) const
        { return static_cast<T>(lookupSymbolImpl(handle_, name)); }
    
    /*!
     * @brief Looks up the address of a specified symbol in the shared library
     * or DLL, and casts it to a string.
     * This is equivalent to calling dlsym() on linux and GetProcAddress()
     * on Windows.
     * \param[in] name Symbol name
     * \return Returns a pointer to the beginning of the string, or an empty
     * string if the symbol could not be found.
     */
    const char* lookupStringSymbol(const char* name) const
    {
        if (auto p = static_cast<const char**>(lookupSymbolImpl(handle_, name)))
            return *p;
        return "";
    }

    /*!
     * \brief Iterates over each symbol present in the shared library's symbol
     * table.
     * \param[in] callback A function or lambda to be called for every symbol
     * name. Return either CONTINUE to continue iterating, STOP to stop iterating,
     * or ERROR if your callback needs to pass an error back to the caller (\see
     * IterateStatus).
     * \return This function returns whatever value the callback function
     * returned last.
     */
    IterateStatus forEachSymbol(std::function<IterateStatus(const char* symbol)> callback) const;

    /*!
     * \brief Iterates over each string resource present in the DLLs resource
     * section. These only exist in Windows DLLs, and are typically created
     * using resource files
     * (see: "Stringtable resources" https://learn.microsoft.com/en-us/windows/win32/menurc/stringtable-resource)
     */
#if defined(_WIN32)
    IterateStatus forEachString(std::function<IterateStatus(const char* str)> callback);
#endif

private:
    void* handle_;
};

}
