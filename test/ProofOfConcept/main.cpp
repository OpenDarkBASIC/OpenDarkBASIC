#include "Windows.h"
#include <vector>
#include <string>
#include <map>
#include <iostream>
using namespace std;

// Typdefs shared by all DLLs
typedef void (*PTR_FuncCreateStr)(DWORD*, DWORD);
typedef DWORD (*PTR_ProcessMessages)(void);
typedef void (*PTR_PrintString)(LPSTR, bool);
typedef void (*PTR_VOID)(void);
typedef void (*PTR_DWORD)(DWORD);

struct GlobStruct {
    char unused[48];
    HINSTANCE g_GFX;
    HINSTANCE g_Text;
    HINSTANCE g_Basic2D;
    HINSTANCE g_Sprites;
    HINSTANCE g_Image;
    HINSTANCE g_Input;
    HINSTANCE g_System;
    HINSTANCE g_File;
    HINSTANCE g_FTP;
    HINSTANCE g_Memblocks;
    HINSTANCE g_Bitmap;
    HINSTANCE g_Animation;
    HINSTANCE g_Multiplayer;
    HINSTANCE g_Basic3D;
    HINSTANCE g_Camera3D;
    HINSTANCE g_Matrix3D;
    HINSTANCE g_Light3D;
    HINSTANCE g_World3D;
    HINSTANCE g_Particles;
    HINSTANCE g_PrimObject;
    HINSTANCE g_Vectors;
    HINSTANCE g_XObject;
    HINSTANCE g_3DSObject;
    HINSTANCE g_MDLObject;
    HINSTANCE g_MD2Object;
    HINSTANCE g_MD3Object;
    HINSTANCE g_Sound;
    HINSTANCE g_Music;
    HINSTANCE g_LODTerrain;
    HINSTANCE g_Q2BSP;
    HINSTANCE g_OwnBSP;
    HINSTANCE g_BSPCompiler;
    HINSTANCE g_CSG;
    HINSTANCE g_igLoader;
    HINSTANCE g_GameFX;
    HINSTANCE g_Transforms;

    // Extra parts of the GlobStruct that we don't care about has been omitted...
};


int main() {
    // Load plugins.
    vector<string> plugins = {
        "DBProCore.dll",
        "DBProImageDebug.dll",
        "DBProSetupDebug.dll",
        "DBProVectorsDebug.dll",
        "DBProBasic2DDebug.dll",
        "DBProMusicDebug.dll",
        "DBProAdvancedMatrixDebug.dll",
        "DBProSpritesDebug.dll",
        "DBProAnimationDebug.dll",
        "DBProFTPDebug.dll",
        "DBProParticlesDebug.dll",
        "DBProMemblocksDebug.dll",
        "DBProTextDebug.dll",
        "DBProBitmapDebug.dll",
        "DBProCameraDebug.dll",
        "DBProBasic3DDebug.dll",
        "DBProFileDebug.dll",
        "DBProMultiplayerPlusDebug.dll",
        "DBProInputDebug.dll",
        "DBProLightDebug.dll",
        "DBProMatrixDebug.dll",
        "DBProMultiplayerDebug.dll",
        "DBProQ3BSPDebug.dll",
        "DBProSoundDebug.dll",
        "DBProSystemDebug.dll",
        "DBProTransformsDebug.dll",
    };
    map<std::string, HMODULE> plugin_handles;
    for (const auto& plugin : plugins) {
        cout << "Loading plugin " << plugin << endl;
        plugin_handles[plugin] = ::LoadLibraryA(plugin.c_str());
        if (plugin_handles[plugin] == NULL) {
            cout << "Failed to load plugin, error: " << ::GetLastError() << endl;
            return 1;
        }
    }

    auto core_dll = plugin_handles["DBProCore.dll"];
    auto basic3d_dll = plugin_handles["DBProBasic3DDebug.dll"];

    // Set up glob struct.
    auto GetGlobPtr = (DWORD (*)())GetProcAddress(core_dll, "?GetGlobPtr@@YAKXZ");
    auto* glob_ptr = (GlobStruct*)GetGlobPtr(); 
    glob_ptr->g_GFX = plugin_handles["DBProSetupDebug.dll"];
    glob_ptr->g_Basic2D = plugin_handles["DBProBasic2DDebug.dll"];
    glob_ptr->g_Text = plugin_handles["DBProTextDebug.dll"];
    glob_ptr->g_Transforms = plugin_handles["DBProTransformsDebug.dll"];
    glob_ptr->g_Sprites = plugin_handles["DBProSpritesDebug.dll"];
    glob_ptr->g_Image = plugin_handles["DBProImageDebug.dll"];
    glob_ptr->g_Input = plugin_handles["DBProInputDebug.dll"];
    glob_ptr->g_System = plugin_handles["DBProSystemDebug.dll"];
    glob_ptr->g_Sound = plugin_handles["DBProSoundDebug.dll"];
    glob_ptr->g_Music = plugin_handles["DBProMusicDebug.dll"];
    glob_ptr->g_File = plugin_handles["DBProFileDebug.dll"];
    glob_ptr->g_FTP = plugin_handles["DBProFTPDebug.dll"];
    glob_ptr->g_Memblocks = plugin_handles["DBProMemblocksDebug.dll"];
    glob_ptr->g_Animation = plugin_handles["DBProAnimationDebug.dll"];
    glob_ptr->g_Bitmap = plugin_handles["DBProBitmapDebug.dll"];
    glob_ptr->g_Multiplayer = plugin_handles["DBProMultiplayerDebug.dll"];
    glob_ptr->g_Camera3D = plugin_handles["DBProCameraDebug.dll"];
    glob_ptr->g_Light3D = plugin_handles["DBProLightDebug.dll"];
    glob_ptr->g_Matrix3D = plugin_handles["DBProMatrixDebug.dll"];
    glob_ptr->g_Basic3D = plugin_handles["DBProBasic3DDebug.dll"];
    glob_ptr->g_World3D = plugin_handles["DBProWorld3DDebug.dll"];
    glob_ptr->g_Q2BSP = plugin_handles["DBProQ2BSPDebug.dll"];
    glob_ptr->g_OwnBSP = plugin_handles["DBProOwnBSPDebug.dll"];
    glob_ptr->g_BSPCompiler = plugin_handles["DBProBSPCompilerDebug.dll"];
    glob_ptr->g_Particles = plugin_handles["DBProParticlesDebug.dll"];
    glob_ptr->g_PrimObject = plugin_handles["DBProPrimObjectDebug.dll"];
    glob_ptr->g_Vectors = plugin_handles["DBProVectorsDebug.dll"];
    glob_ptr->g_LODTerrain = plugin_handles["DBProLODTerrainDebug.dll"];
    glob_ptr->g_CSG = plugin_handles["DBProCSGDebug.dll"];

    // Initialise engine.
    auto PassCmdLinePtr = (void(*)(void*))GetProcAddress(core_dll, "?PassCmdLineHandlerPtr@@YAXPAX@Z");
    auto PassErrorPtr = (void(*)(void*))GetProcAddress(core_dll, "?PassErrorHandlerPtr@@YAXPAX@Z");
    auto InitDisplay = (DWORD(*)(DWORD, DWORD, DWORD, DWORD, HINSTANCE, char*))GetProcAddress(core_dll, "?InitDisplay@@YAKKKKKPAUHINSTANCE__@@PAD@Z");
    auto CloseDisplay = (DWORD(*)())GetProcAddress(core_dll, "?CloseDisplay@@YAKXZ");
    auto PassDLLs = (void(*)())GetProcAddress(core_dll, "?PassDLLs@@YAXXZ");

    char cmdline_ptr[] = "command_line";
    char app_name[] = "test_app";
    DWORD runtime_error = 0;
    DWORD initial_display_mode = 1;
    DWORD initial_display_width = 640;
    DWORD initial_display_height = 480;
    DWORD initial_display_depth = 32;

    /*
    I think this is needed for non TGC dlls...

    for (const auto& [name, handle] : plugin_handles) {
        auto PassCoreData = (void (*)(void*))GetProcAddress(handle, "?PassCoreData@@YAXPAX@Z");
        if (PassCoreData) {
            PassCoreData(globPtr);
        }
    }
    */

//    PassCmdLinePtr((void*)cmdline_ptr);
    PassErrorPtr((void*)&runtime_error);
    PassDLLs();
    if (InitDisplay(initial_display_mode, initial_display_width, initial_display_height,
                    initial_display_depth, ::GetModuleHandle(NULL), nullptr) != 0) {
        cout << "Failed to initialise display." << endl;
        return 1;
    }

    // Run some commands.
    // auto Print = (void (*)(char*))GetProcAddress(core_dll, "?PrintS@@YAXPAD@Z");
    auto Sync = (void (*)())GetProcAddress(core_dll, "?Sync@@YAXXZ");
    auto MakeObjectCube = (void (*)(int, float))GetProcAddress(basic3d_dll, "?MakeCube@@YAXHM@Z");
    auto YRotateObject = (void (*)(int, float))GetProcAddress(basic3d_dll, "?YRotate@@YAXHM@Z");

    MakeObjectCube(1, 10.0f);
    float angle = 0.0f;
    while (true) {
        angle += 0.02f;
        YRotateObject(1, angle);
        Sync();
    }

    CloseDisplay();

	return 0;
}