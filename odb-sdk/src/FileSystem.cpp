#include "odb-sdk/FileSystem.hpp"
#include "odb-sdk/Str.hpp"

#if defined(ODBSDK_PLATFORM_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   include <io.h>
#   include <Windows.h>
#elif defined(ODBSDK_PLATFORM_MACOS)
#   include <mach-o/dyld.h>
#   include <limits.h>
#elif defined(ODBSDK_PLATFORM_LINUX)
#   include <limits.h>  // PATH_MAX
#   include <unistd.h>  // readlink
#else
#   error "Platform not supported"
#endif

namespace fs = std::filesystem;

namespace odb {

// ----------------------------------------------------------------------------
FILE* FileSystem::dupFilePointer(FILE* file)
{
#if defined(ODBSDK_PLATFORM_LINUX)
    FILE* newFp;
    int fd = fileno(file);
    int newFd = dup(fd);
    if (newFd == -1)
        goto dup_failed;
    newFp = fdopen(newFd, "r");
    if (newFp == nullptr)
        goto fdopen_failed;

    return newFp;

    fdopen_failed : close(newFd);
    dup_failed    : return nullptr;
#elif defined(ODBSDK_PLATFORM_MACOS)
    // TODO
    return nullptr;
#elif defined(ODBSDK_PLATFORM_WIN32)
    FILE* newFp;
    int fd = _fileno(file);
    int newFd = _dup(fd);
    if (newFd == -1)
        goto dup_failed;
    newFp = _fdopen(newFd, "r");
    if (newFp == nullptr)
        goto fdopen_failed;

    return newFp;

    fdopen_failed : _close(newFd);
    dup_failed    : return nullptr;
#endif
}

// ----------------------------------------------------------------------------
bool FileSystem::isDynamicLib(const fs::path& filename)
{
    std::string ext = str::toLower(filename.extension().string());
    // TODO probably a more reliable way is to dlopen() and see if it loads
    return ext == ".dll"
        || ext == ".so"
        || ext == ".dylib"
        || ext == ".dynlib";
}

// ----------------------------------------------------------------------------
fs::path FileSystem::getPathToSelf()
{
    auto getPlatSpecificPath = []() -> fs::path {
#if defined(ODBSDK_PLATFORM_LINUX)
        char path[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
        return std::string(path, (count > 0) ? count : 0);
#elif defined(ODBSDK_PLATFORM_MACOS)
        char path[PATH_MAX];
        uint32_t bufsize = PATH_MAX;
        if(_NSGetExecutablePath(path, &bufsize) == 0)
            return std::string(path, bufsize);
        return "";
#elif defined(ODBSDK_PLATFORM_WIN32)
        char path[_MAX_PATH] = { 0 };
        GetModuleFileNameA(NULL, path, MAX_PATH);
        return path;
#endif
    };

    return fs::canonical(getPlatSpecificPath());
}

}
