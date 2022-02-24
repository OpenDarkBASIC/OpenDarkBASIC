#include <unordered_map>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdarg>
#include <cstdio>
#include <iostream>

#include "globstruct.h"

#define DLLEXPORT __declspec(dllexport)

namespace {
std::unordered_map<std::string, HMODULE> loadedPlugins;

std::string GetLastErrorAsString()
{
    // Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return "";
    }

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}
}

extern "C" {
DLLEXPORT void* loadPlugin(const char* pluginName) {
    HMODULE library = LoadLibraryA(pluginName);
    if (!library) {
        printf("Failed to load plugin %s. Reason: %s", pluginName, GetLastErrorAsString().c_str());
        return nullptr;
    }
    loadedPlugins.emplace(pluginName, library);
    return static_cast<void*>(library);
}

DLLEXPORT void* getFunctionAddress(void* plugin, const char* functionName) {
    return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(plugin), functionName));
}

DLLEXPORT void debugPrintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    (void)vprintf_s(fmt, args);
    va_end(args);
}

DLLEXPORT int initialiseEngine() {
    auto core_dll = loadedPlugins["DBProCore.dll"];

    // Set up glob struct.
    auto GetGlobPtr = (DWORD (*)())GetProcAddress(core_dll, "?GetGlobPtr@@YAKXZ");
    auto* glob_ptr = reinterpret_cast<GlobStruct*>(GetGlobPtr());
    glob_ptr->g_GFX = loadedPlugins["DBProSetupDebug.dll"];
    glob_ptr->g_Basic2D = loadedPlugins["DBProBasic2DDebug.dll"];
    glob_ptr->g_Text = loadedPlugins["DBProTextDebug.dll"];
    glob_ptr->g_Transforms = loadedPlugins["DBProTransformsDebug.dll"];
    glob_ptr->g_Sprites = loadedPlugins["DBProSpritesDebug.dll"];
    glob_ptr->g_Image = loadedPlugins["DBProImageDebug.dll"];
    glob_ptr->g_Input = loadedPlugins["DBProInputDebug.dll"];
    glob_ptr->g_System = loadedPlugins["DBProSystemDebug.dll"];
    glob_ptr->g_Sound = loadedPlugins["DBProSoundDebug.dll"];
    glob_ptr->g_Music = loadedPlugins["DBProMusicDebug.dll"];
    glob_ptr->g_File = loadedPlugins["DBProFileDebug.dll"];
    glob_ptr->g_FTP = loadedPlugins["DBProFTPDebug.dll"];
    glob_ptr->g_Memblocks = loadedPlugins["DBProMemblocksDebug.dll"];
    glob_ptr->g_Animation = loadedPlugins["DBProAnimationDebug.dll"];
    glob_ptr->g_Bitmap = loadedPlugins["DBProBitmapDebug.dll"];
    glob_ptr->g_Multiplayer = loadedPlugins["DBProMultiplayerDebug.dll"];
    glob_ptr->g_Camera3D = loadedPlugins["DBProCameraDebug.dll"];
    glob_ptr->g_Light3D = loadedPlugins["DBProLightDebug.dll"];
    glob_ptr->g_Matrix3D = loadedPlugins["DBProMatrixDebug.dll"];
    glob_ptr->g_Basic3D = loadedPlugins["DBProBasic3DDebug.dll"];
    glob_ptr->g_World3D = loadedPlugins["DBProWorld3DDebug.dll"];
    glob_ptr->g_Q2BSP = loadedPlugins["DBProQ2BSPDebug.dll"];
    glob_ptr->g_OwnBSP = loadedPlugins["DBProOwnBSPDebug.dll"];
    glob_ptr->g_BSPCompiler = loadedPlugins["DBProBSPCompilerDebug.dll"];
    glob_ptr->g_Particles = loadedPlugins["DBProParticlesDebug.dll"];
    glob_ptr->g_PrimObject = loadedPlugins["DBProPrimObjectDebug.dll"];
    glob_ptr->g_Vectors = loadedPlugins["DBProVectorsDebug.dll"];
    glob_ptr->g_LODTerrain = loadedPlugins["DBProLODTerrainDebug.dll"];
    glob_ptr->g_CSG = loadedPlugins["DBProCSGDebug.dll"];

    // Initialise engine.
    // auto PassCmdLinePtr = (void(*)(void*))GetProcAddress(core_dll, "?PassCmdLineHandlerPtr@@YAXPAX@Z");
    auto PassErrorPtr = (void(*)(void*))GetProcAddress(core_dll, "?PassErrorHandlerPtr@@YAXPAX@Z");
    auto InitDisplay = (DWORD(*)(DWORD, DWORD, DWORD, DWORD, HINSTANCE, char*))GetProcAddress(core_dll, "?InitDisplay@@YAKKKKKPAUHINSTANCE__@@PAD@Z");
    // auto CloseDisplay = (DWORD(*)())GetProcAddress(core_dll, "?CloseDisplay@@YAKXZ");
    auto PassDLLs = (void(*)())GetProcAddress(core_dll, "?PassDLLs@@YAXXZ");

    char cmdlinePtr[] = "command_line";
    char app_name[] = "test_app";
    DWORD runtimeError = 0;
    DWORD initialDisplayMode = 1;
    DWORD initialDisplayWidth = 640;
    DWORD initialDisplayHeight = 480;
    DWORD initialDisplayDepth = 32;

    int tempPathSize = GetTempPathA(260, glob_ptr->pEXEUnpackDirectory);
    char unpackDir[] = "odbc-unpack";
    memcpy(glob_ptr->pEXEUnpackDirectory + tempPathSize, unpackDir, sizeof(unpackDir));

    /*
    I think this is needed for non TGC dlls...

    for (const auto& [name, handle] : loadedPlugins) {
        auto PassCoreData = (void (*)(void*))GetProcAddress(handle, "?PassCoreData@@YAXPAX@Z");
        if (PassCoreData) {
            PassCoreData(globPtr);
        }
    }
    */

    // PassCmdLinePtr((void*)cmdlinePtr);
    PassErrorPtr(reinterpret_cast<void*>(&runtimeError));
    PassDLLs();
    if (InitDisplay(initialDisplayMode, initialDisplayWidth, initialDisplayHeight,
                    initialDisplayDepth, ::GetModuleHandleA(nullptr), nullptr) != 0) {
        std::cout << "Failed to initialise display." << std::endl;
        return 1;
    }

    return 0;
}
}