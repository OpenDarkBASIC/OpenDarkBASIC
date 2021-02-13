#include "odb-sdk/FileSystem.hpp"

#if defined(ODBSDK_PLATFORM_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <io.h>
#elif defined(ODBSDK_PLATFORM_MACOS) || defined(ODBSDK_PLATFORM_LINUX)
#include <limits.h>  // PATH_MAX
#include <unistd.h>  // readlink
#endif

namespace fs = std::filesystem;

namespace odb {

// ----------------------------------------------------------------------------
FILE* FileSystem::dupFilePointer(FILE* file)
{
#if defined(ODBSDK_PLATFORM_WIN32)
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
#elif defined(ODBSDK_PLATFORM_MACOS) || defined(ODBSDK_PLATFORM_LINUX)
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
#endif
}

// ----------------------------------------------------------------------------
bool FileSystem::isDynamicLib(const std::filesystem::path& filename)
{
    // TODO probably a more reliable way is to dlopen() and see if it loads
    return filename.extension() == ".dll"
        || filename.extension() == ".so";
}

// ----------------------------------------------------------------------------
std::filesystem::path FileSystem::getPathToSelf()
{
#if defined(ODBSDK_PLATFORM_WIN32)
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return path;
#elif defined(ODBSDK_PLATFORM_LINUX)
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return std::string(result, (count > 0) ? count : 0);
#endif
}

}
