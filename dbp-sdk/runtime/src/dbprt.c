#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "dbp-runtime/config.h"
#include "dbp-runtime/globstruct.h"
#include <stdio.h>

static HMODULE coreDLL;
static struct GlobStruct* globPtr;
static DWORD last_dbp_error;

#define DBPPATH "C:\\Program Files (x86)\\The Game Creators\\Dark Basic Professional Online\\Compiler\\plugins\\"

#define DBP_PLUGIN_LIST \
    X(g_GFX,         "DBProSetupDebug.dll") \
    X(g_Basic2D,     "DBProBasic2DDebug.dll") \
    X(g_Text,        "DBProTextDebug.dll") \
    X(g_Transforms,  "DBProTransformsDebug.dll") \
    X(g_Sprites,     "DBProSpritesDebug.dll") \
    X(g_Image,       "DBProImageDebug.dll") \
    X(g_Input,       "DBProInputDebug.dll") \
    X(g_System,      "DBProSystemDebug.dll") \
    X(g_Sound,       "DBProSoundDebug.dll") \
    X(g_Music,       "DBProMusicDebug.dll") \
    X(g_File,        "DBProFileDebug.dll") \
    X(g_FTP,         "DBProFTPDebug.dll") \
    X(g_Memblocks,   "DBProMemblocksDebug.dll") \
    X(g_Animation,   "DBProAnimationDebug.dll") \
    X(g_Bitmap,      "DBProBitmapDebug.dll") \
    X(g_Multiplayer, "DBProMultiplayerDebug.dll") \
    X(g_Camera3D,    "DBProCameraDebug.dll") \
    X(g_Light3D,     "DBProLightDebug.dll") \
    X(g_Matrix3D,    "DBProMatrixDebug.dll") \
    X(g_Basic3D,     "DBProBasic3DDebug.dll") \
    X(g_World3D,     "DBProWorld3DDebug.dll") \
    X(g_Q2BSP,       "DBProQ2BSPDebug.dll") \
    X(g_OwnBSP,      "DBProOwnBSPDebug.dll") \
    X(g_BSPCompiler, "DBProBSPCompilerDebug.dll") \
    X(g_Particles,   "DBProParticlesDebug.dll") \
    /*X(g_PrimObject, "DBProPrimObjectDebug.dll")*/ \
    X(g_Vectors,     "DBProVectorsDebug.dll") \
    X(g_LODTerrain,  "DBProLODTerrainDebug.dll") \
    /* X(g_CSG, "DBProCSGDebug.dll")*/

static void
log_last_error(void)
{
    LPTSTR error;
    DWORD dwError = GetLastError();
    if (!FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dwError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&error,
            0,
            NULL))
    {
        printf("(%d)\n", dwError);
        return;
    }

    puts(error);
    LocalFree(error);
}

static HMODULE load_dbp_plugin(const char* path)
{
    HMODULE dll = LoadLibraryA(path);
    if (dll == NULL)
    {
        printf("Failed to load %s: ", path);
        log_last_error();
    }
    return dll;
}

DBPRUNTIME_API int
odbrt_init(void)
{
    /*
        Based upon CEXEBlock::Init.
     */

    // Set up glob struct.
    coreDLL = load_dbp_plugin("DBProCore.dll");
    if (coreDLL == NULL)
        return -1;
    DWORD(*GetGlobPtr)(void) = (DWORD(*)(void))GetProcAddress(coreDLL, "?GetGlobPtr@@YAKXZ");
    globPtr = (struct GlobStruct*)(intptr_t)GetGlobPtr();

#define X(g, name) \
    if ((globPtr->g = load_dbp_plugin(DBPPATH name)) == NULL) return -1;
    DBP_PLUGIN_LIST
#undef X

    char cmdlinePtr[] = "command_line";
    char appName[] = "test_app";
    DWORD initialDisplayMode = 1;
    DWORD initialDisplayWidth = 640;
    DWORD initialDisplayHeight = 480;
    DWORD initialDisplayDepth = 32;

    memset(globPtr->pEXEUnpackDirectory, 0, sizeof(globPtr->pEXEUnpackDirectory));
    int tempPathSize = GetTempPathA(MAX_PATH, globPtr->pEXEUnpackDirectory);
    char unpackDir[] = "odb-unpack";
    memcpy(globPtr->pEXEUnpackDirectory + tempPathSize, unpackDir, sizeof(unpackDir));
    
    void (*PassCmdLinePtr)(void*) = (void(*)(void*))GetProcAddress(coreDLL, "?PassCmdLineHandlerPtr@@YAXPAX@Z");
    PassCmdLinePtr((void*)cmdlinePtr);

    void (*PassErrorHandlerPtr)(void*) = (void (*)(void*))GetProcAddress(coreDLL, "?PassErrorHandlerPtr@@YAXPAX@Z");
    PassErrorHandlerPtr((void*)&last_dbp_error);

    // Causes the Core DLL to load all of its function pointers from all of the other DLLs
    // (such as g_GFX_Constructor), which means this needs to be called before ConstructDLLs().
    void (*PassDLLs)(void) = (void (*)(void))GetProcAddress(coreDLL, "?PassDLLs@@YAXXZ");
    PassDLLs();
    
    DWORD (*InitDisplay)(DWORD, DWORD, DWORD, DWORD, HINSTANCE, char*) = 
        (DWORD(*)(DWORD, DWORD, DWORD, DWORD, HINSTANCE, char*))GetProcAddress(coreDLL, "?InitDisplay@@YAKKKKKPAUHINSTANCE__@@PAD@Z");
    if (InitDisplay(initialDisplayMode, initialDisplayWidth, initialDisplayHeight, initialDisplayDepth,
                    GetModuleHandleA(NULL), "OpenDarkBASIC application") != 0)
    {
        puts("Failed to init display.");
        return -1;
    }

    // Pass core data to plugins.
#define X(g, name) { \
    void (*ReceiveCoreDataPtr)(void*) = (void(*)(void*))GetProcAddress(globPtr->g, "?ReceiveCoreDataPtr@@YAXPAX@Z"); \
    if (ReceiveCoreDataPtr == NULL) \
        ReceiveCoreDataPtr = (void(*)(void*))GetProcAddress(globPtr->g, "ReceiveCoreDataPtr"); \
    if (ReceiveCoreDataPtr) \
        ReceiveCoreDataPtr(globPtr); \
    }
    //DBP_PLUGIN_LIST
#undef X
    
    // ConstructDLLs will go through each DLL entry in the globstruct (e.g. g_Image, g_Basic2D, etc.)
    // and call the functions:
    //    1) Constructor()
    //    2) PassErrorHandler()
    //    3) PassCoreData()
    void (*ConstructDLLs)(void) = (void (*)(void))GetProcAddress(coreDLL, "?ConstructDLLs@@YAXXZ");
    ConstructDLLs();

    return 0;
}

DBPRUNTIME_API void
odbrt_exit(void)
{
    DWORD (*CloseDisplay)(void) = (DWORD(*)(void))GetProcAddress(coreDLL, "?CloseDisplay@@YAKXZ");
    if (CloseDisplay() != 0)
        puts("Failed to close display.");
    ExitProcess(0);
}
