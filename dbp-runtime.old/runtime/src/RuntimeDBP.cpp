#include <string>
#include <unordered_map>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdarg>
#include <cstdio>
#include <iostream>

#include "globstruct.h"

#define DLLEXPORT __declspec(dllexport)

namespace {
std::unordered_map<std::string, HMODULE> loadedPlugins;
DWORD runtimeError = 0;
GlobStruct* globPtr = nullptr;

std::string GetLastErrorAsString()
{
    // Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
    {
        return "";
    }

    LPSTR messageBuffer = nullptr;
    size_t size =
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}
} // namespace

extern "C" {
DLLEXPORT void* loadPlugin(const char* pluginName)
{
    HMODULE library = LoadLibraryA(pluginName);
    if (!library)
    {
        char buffer[255];
        sprintf_s(buffer, 255, "Failed to load plugin %s. Reason: %s", pluginName, GetLastErrorAsString().c_str());
        puts(buffer);
        MessageBoxA(NULL, buffer, "Fatal error", 0);
        return nullptr;
    }
    loadedPlugins.emplace(pluginName, library);

    // Call constructor (if any).
    auto Constructor = (void (*)())GetProcAddress(library, "?Constructor@@YAXXZZ");
    if (!Constructor)
    {
        Constructor = (void (*)())GetProcAddress(library, "Constructor");
    }
    if (Constructor)
    {
        Constructor();
    }

    return static_cast<void*>(library);
}

DLLEXPORT void* getFunctionAddress(void* plugin, const char* functionName)
{
    auto address = GetProcAddress(static_cast<HMODULE>(plugin), functionName);
    if (address == nullptr)
    {
        char pluginBuffer[255];
        GetModuleFileNameA(static_cast<HMODULE>(plugin), pluginBuffer, 255);
        char buffer[255];
        sprintf_s(buffer, 255, "Failed to load function %s from plugin %s.", functionName, pluginBuffer);
        puts(buffer);
        MessageBoxA(NULL, buffer, "Fatal error", 0);
        return nullptr;
    }
    return reinterpret_cast<void*>(address);
}

DLLEXPORT void debugPrintf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    (void)vprintf_s(fmt, args);
    va_end(args);
}

DLLEXPORT int initEngine()
{
    /*
        Based upon CEXEBlock::Init.
     */

    auto coreDll = loadedPlugins["DBProCore.dll"];

    // Set up glob struct.
    auto GetGlobPtr = (DWORD(*)())GetProcAddress(coreDll, "?GetGlobPtr@@YAKXZ");
    globPtr = reinterpret_cast<GlobStruct*>(GetGlobPtr());
    globPtr->g_GFX = loadedPlugins["DBProSetupDebug.dll"];
    globPtr->g_Basic2D = loadedPlugins["DBProBasic2DDebug.dll"];
    globPtr->g_Text = loadedPlugins["DBProTextDebug.dll"];
    globPtr->g_Transforms = loadedPlugins["DBProTransformsDebug.dll"];
    globPtr->g_Sprites = loadedPlugins["DBProSpritesDebug.dll"];
    globPtr->g_Image = loadedPlugins["DBProImageDebug.dll"];
    globPtr->g_Input = loadedPlugins["DBProInputDebug.dll"];
    globPtr->g_System = loadedPlugins["DBProSystemDebug.dll"];
    globPtr->g_Sound = loadedPlugins["DBProSoundDebug.dll"];
    globPtr->g_Music = loadedPlugins["DBProMusicDebug.dll"];
    globPtr->g_File = loadedPlugins["DBProFileDebug.dll"];
    globPtr->g_FTP = loadedPlugins["DBProFTPDebug.dll"];
    globPtr->g_Memblocks = loadedPlugins["DBProMemblocksDebug.dll"];
    globPtr->g_Animation = loadedPlugins["DBProAnimationDebug.dll"];
    globPtr->g_Bitmap = loadedPlugins["DBProBitmapDebug.dll"];
    globPtr->g_Multiplayer = loadedPlugins["DBProMultiplayerDebug.dll"];
    globPtr->g_Camera3D = loadedPlugins["DBProCameraDebug.dll"];
    globPtr->g_Light3D = loadedPlugins["DBProLightDebug.dll"];
    globPtr->g_Matrix3D = loadedPlugins["DBProMatrixDebug.dll"];
    globPtr->g_Basic3D = loadedPlugins["DBProBasic3DDebug.dll"];
    globPtr->g_World3D = loadedPlugins["DBProWorld3DDebug.dll"];
    globPtr->g_Q2BSP = loadedPlugins["DBProQ2BSPDebug.dll"];
    globPtr->g_OwnBSP = loadedPlugins["DBProOwnBSPDebug.dll"];
    globPtr->g_BSPCompiler = loadedPlugins["DBProBSPCompilerDebug.dll"];
    globPtr->g_Particles = loadedPlugins["DBProParticlesDebug.dll"];
    globPtr->g_PrimObject = loadedPlugins["DBProPrimObjectDebug.dll"];
    globPtr->g_Vectors = loadedPlugins["DBProVectorsDebug.dll"];
    globPtr->g_LODTerrain = loadedPlugins["DBProLODTerrainDebug.dll"];
    globPtr->g_CSG = loadedPlugins["DBProCSGDebug.dll"];

    // Init engine.
    // auto PassCmdLinePtr = (void(*)(void*))GetProcAddress(coreDll, "?PassCmdLineHandlerPtr@@YAXPAX@Z");
    auto PassErrorPtr = (void (*)(void*))GetProcAddress(coreDll, "?PassErrorHandlerPtr@@YAXPAX@Z");
    auto InitDisplay = (DWORD(*)(DWORD, DWORD, DWORD, DWORD, HINSTANCE, char*))GetProcAddress(
        coreDll, "?InitDisplay@@YAKKKKKPAUHINSTANCE__@@PAD@Z");
    auto PassDLLs = (void (*)())GetProcAddress(coreDll, "?PassDLLs@@YAXXZ");
    auto ConstructDLLs = (void (*)())GetProcAddress(coreDll, "?ConstructDLLs@@YAXXZ");

    char cmdlinePtr[] = "command_line";
    char appName[] = "test_app";
    DWORD initialDisplayMode = 1;
    DWORD initialDisplayWidth = 640;
    DWORD initialDisplayHeight = 480;
    DWORD initialDisplayDepth = 32;

    memset(globPtr->pEXEUnpackDirectory, 0, sizeof(globPtr->pEXEUnpackDirectory));
    int tempPathSize = GetTempPathA(260, globPtr->pEXEUnpackDirectory);
    char unpackDir[] = "odbc-unpack";
    memcpy(globPtr->pEXEUnpackDirectory + tempPathSize, unpackDir, sizeof(unpackDir));

    // PassCmdLinePtr((void*)cmdlinePtr);
    PassErrorPtr(reinterpret_cast<void*>(&runtimeError));
    PassDLLs();
    if (InitDisplay(initialDisplayMode, initialDisplayWidth, initialDisplayHeight, initialDisplayDepth,
                    ::GetModuleHandleA(nullptr), "OpenDarkBASIC application") != 0)
    {
        std::cout << "Failed to init display." << std::endl;
        return 1;
    }

    // Pass core data to plugins.
    for (const auto& entry : loadedPlugins)
    {
        auto PassCoreData = (void (*)(void*))GetProcAddress(entry.second, "?ReceiveCoreDataPtr@@YAXPAX@Z");
        if (!PassCoreData)
        {
            PassCoreData = (void (*)(void*))GetProcAddress(entry.second, "ReceiveCoreDataPtr");
        }
        if (PassCoreData)
        {
            PassCoreData(globPtr);
        }
    }

    ConstructDLLs();

    return 0;
}

DLLEXPORT void checkArrayBounds(void* arrayPtr, DWORD lineNumber, DWORD d1, DWORD d2, DWORD d3, DWORD d4, DWORD d5,
                                DWORD d6, DWORD d7, DWORD d8, DWORD d9)
{
    /*
        header[0] = d1
        header[1] = header[0] * d2
        header[2] = header[1] * d3
        header[3] = header[2] * d4
        header[4] = header[3] * d5
        header[5] = header[4] * d6
        header[6] = header[5] * d7
        header[7] = header[6] * d8
        header[8] = header[7] * d9
        header[9] = ...
        header[10] = N
        header[11] = size of one item, encoded in the 2nd parameter of DimDDD
        header[12] = type ID of each item, encoded in the 2nd parameter of DimDDD;
        header[13] = the internal index of this array (what 'array()' means), defaults to 0
    */
    DWORD* header = static_cast<DWORD*>(arrayPtr) - 14;
    bool fail = false;
    if (header[1] == 0)
    {
        // If we have a single dimensional array, check the total size
        if (d1 > header[10])
        {
            fail = true;
        }
    }
    else
    {
        if (d1 > header[0])
        {
            fail = true;
        }
    }
    if (fail)
    {
        char err[512];
        wsprintf(err, "OpenDarkBASIC (DBPro) has detected an unexpected issue and needs to restart your session.\n\nArray index %d out of bounds of array of size %d on line %d.",
                 d1, header[0], lineNumber);
        MessageBox(nullptr, err, "OpenDarkBASIC (DBPro) Problem Detected", MB_TOPMOST | MB_OK);
        ExitProcess(1);
    }
}

DLLEXPORT void checkForError(const char* commandName, DWORD lineNumber)
{
    static std::unordered_map<DWORD, const char*> runtimeCodeStrings = {
        {0, "An unknown error has occurred"},
        {1, "Broke from nested subroutine. Cannot resume program after CLI usage."},
        {21, "The program tried to execute a function header declaration"},
        {51, "The index number you have used is bigger than the array size"},
        {52, "The index number you have used must be within the arrray"},
        {53, "The array is empty"},
        {54, "Array type is invalid"},
        {55, "Array specified must be a single dimension array for this type of command"},
        {101, "There was not enough memory to make such an array"},
        {102, "Unknown array error"},
        {103, "The file is too large"},
        {104, "The file is invalid"},
        {105, "File does not exist"},
        {106, "File already exists"},
        {107, "The string is to long"},
        {108, "Stack overflow error"},
        {109, "Cannot make an array of that type"},
        {110, "Only positive numbers are allowed"},
        {111, "Divide by zero error"},
        {112, "Illegal size error"},
        {113, "Only positive numbers are allowed"},
        {114, "You must use a variable"},
        {115, "You must use a variable"},
        {116, "This command is now obsolete"},
        {117, "Cannot access the file because it is being used by another process"},
        {118, "Array does not exist or array subscript out of bounds"},
        {119, "Division by zero"},
        {300, "Unknown sprite error"},
        {301, "Sprite number must be greater than zero"},
        {302, "Sprite does not exist"},
        {303, "Sprite backsave illegal"},
        {304, "Sprite transparency illegal"},
        {310, "Sprite rotation illegal"},
        {311, "Sprite scale illegal"},
        {312, "Sprite size illegal"},
        {313, "Sprite angle illegal"},
        {314, "Sprite alpha value illegal"},
        {315, "Sprite rgb value illegal"},
        {316, "Sprite animation delay value illegal"},
        {317, "Sprite vertex number illegal"},
        {318, "Sprite width illegal"},
        {319, "Sprite height illegal"},
        {320, "Sprite animation value illegal"},
        {321, "Sprite already exists"},
        {500, "Unknown image error"},
        {501, "Image number illegal"},
        {502, "Image does not exist"},
        {503, "Cannot grab image due to the area being too large"},
        {504, "Cannot grab image due to an illegal area"},
        {505, "Image is too large to be a texture"},
        {506, "Could not load image"},
        {507, "Cannot read an image currently locked by system"},
        {1000, "Unknown bitmap error"},
        {1001, "Bitmap number illegal"},
        {1002, "Bitmap does not exist"},
        {1003, "Could not load bitmap"},
        {1004, "Could not save bitmap"},
        {1005, "Could not create bitmap"},
        {1006, "Cannot delete bitmap zerp"},
        {1007, "Bitmap numbers are identical"},
        {1008, "Source bitmap is too large"},
        {1009, "Bitmap zero is reserved"},
        {1010, "Region has exceeded bitmap size"},
        {1011, "Bitmap area is illegal"},
        {1012, "Blur value illegal"},
        {1013, "Fade value illegal"},
        {1014, "Gamm value illegal"},
        {1500, "Unknown screen error"},
        {1501, "Screen size is illegal"},
        {1502, "Screen depth is illegal"},
        {1503, "Screen mode is illegal"},
        {1504, "Display cannot be created due to unknown DirectX error"},
        {1505, "Display using 16 bit is not supported by available hardware"},
        {1506, "Display using 24 bit is not supported by available hardware"},
        {1507, "Display using 32 bit is not supported by available hardware"},
        {1508, "Display does not support selected method of vertex processing"},
        {1509, "Display does not support selected method of backbuffer access"},
        {1510, "Display cannot be initialized due to absent DirectX interface"},
        {1511, "Display cannot be created due to invalid function parameters"},
        {1512, "Display is not supported by available hardware"},
        {1513, "Display cannot be created due to insufficient video memory"},
        {1514, "24 bit depth not supported. Select either 16 or 32 bit mode"},
        {2000, "Unknown animation error"},
        {2001, "Animation number illegal"},
        {2002, "Could not load animation"},
        {2003, "Animation already exists"},
        {2004, "Animation does not exist"},
        {2005, "Animation volume value illegal"},
        {2006, "Animation frequency value illegal"},
        {2007, "Animation is already playing"},
        {2008, "Animation is not playing"},
        {2009, "Animation is already paused"},
        {2010, "Animation is not paused"},
        {3000, "Unknown sound error"},
        {3001, "Sound number illegal"},
        {3002, "Could not load sound"},
        {3003, "Sound already exists"},
        {3004, "Sound does not exist"},
        {3005, "Sound volume value illegal"},
        {3006, "Sound frequency value illegal"},
        {3007, "Sound pan value illegal"},
        {3008, "Could not save sound"},
        {3021, "Could not clone sound"},
        {3022, "A sound must be specified"},
        {3023, "A sound must be specified to clone"},
        {3201, "A speech engine cannot be found"},
        {3500, "Unknown music error"},
        {3501, "Music number illegal"},
        {3502, "Could not load music"},
        {3503, "Music already exists"},
        {3504, "Music does not exist"},
        {3505, "Music volume value illegal"},
        {3506, "Music speed value illegal"},
        {3507, "Music is not playing"},
        {3508, "Music track is illegal"},
        {4001, "Unknown controller error"},
        {4002, "A controller has not been selected"},
        {4101, "No force-feedback controller found"},
        {4102, "Magnitude value illegal"},
        {4103, "Duration value illegal"},
        {4104, "Angle value illegal"},
        {5101, "Memblock range is illegal"},
        {5102, "Memblock already exists"},
        {5103, "Memblock does not exist"},
        {5104, "Could not create memblock"},
        {5105, "Memblock position outside range"},
        {5106, "Memblock size value illegal"},
        {5107, "Not a memblock byte"},
        {5108, "Not a memblock word"},
        {5109, "Not a memblock dword"},
        {7000, "Unknown 3D error"},
        {7001, "Mesh number illegal"},
        {7002, "Could not load mesh"},
        {7003, "Mesh does not exist"},
        {7004, "Mesh darken value illegal"},
        {7005, "Mesh lighten value illegal"},
        {7006, "Object number illegal"},
        {7007, "Object already exists"},
        {7008, "Object does not exist"},
        {7009, "Matrix already exists"},
        {7010, "Matrix number illegal"},
        {7011, "Matrix dimensions illegal"},
        {7012, "Matrix segments illegal"},
        {7013, "Matrix does not exist"},
        {7014, "Matrix height error"},
        {7015, "Matrix exceeds maximum size"},
        {7016, "Matrix tile illegal"},
        {7017, "Matrix coordinates illegal"},
        {7018, "Could not load 3D object"},
        {7019, "Object angle value illegal"},
        {7020, "Too many limbs exist in the object"},
        {7021, "Limb number illegal"},
        {7022, "Cannot add a limb with that number"},
        {7023, "Limb does not exist"},
        {7024, "Limb already exists"},
        {7025, "Cannot link a limb to that number"},
        {7026, "You must link limbs in chain sequence"},
        {7027, "You can only link new limbs"},
        {7028, "Mesh is too large for limb placement"},
        {7029, "3D Memory Error"},
        {7030, "You must use the .DBO extension for a saved DBPro object"},
        {7031, "Object fade value illegal"},
        {7032, "Limb does not contain a mesh"},
        {7101, "Particles object already exists"},
        {7102, "Particles object does not exist"},
        {7103, "Unknown particles objects error"},
        {7104, "Particle number illegal"},
        {7105, "Particle must have an emission value greater than zero"},
        {7126, "Could not create terrain"},
        {7127, "Terrain already exists"},
        {7128, "Terrain does not exist"},
        {7129, "Terrain data must be exactly square"},
        {7130, "Terrain number illegal"},
        {7151, "Could not load BSP file"},
        {7152, "BSP does not exist"},
        {7153, "BSP already exists"},
        {7154, "BSP collision index must be between 1 and 24"},
        {7201, "Camera number illegal"},
        {7202, "Camera already exists"},
        {7203, "Camera does not exist"},
        {7204, "Cannot create camera"},
        {7205, "Cannot use camera zero"},
        {7301, "Light number illegal"},
        {7302, "Light already exists"},
        {7303, "Light does not exist"},
        {7304, "Cannot create light	"},
        {7305, "Cannot use light zero"},
        {7601, "Animation frame number illegal"},
        {7602, "Animation keyframe does not exist"},
        {7603, "Animation speed value illegal"},
        {7604, "Animation interpolation illegal"},
        {7605, "Cannot append to the object"},
        {7606, "Append can only add to end of object animation"},
        {7607, "Failed to compile CSG file from X File"},
        {7701, "Vertex shader number illegal"},
        {7702, "Vertex shader count illegal"},
        {7703, "Cannot create vertex shader"},
        {7704, "Vertex shader stream position illegal"},
        {7705, "Vertex shader data illegal"},
        {7706, "Vertex shader stream illegal"},
        {7707, "Vertex shader cannot be assembled"},
        {7708, "Vertex shader is illegal"},
        {7721, "Effect number is illegal"},
        {7722, "Effect does not exist"},
        {7723, "Effect already exists"},
        {7801, "Vector does not exist"},
        {7802, "Matrix4 does not exist"},
        {7901, "Fogging effect is not available"},
        {7911, "Ambient percentage value illegal"},
        {7912, "Camera range value illegal"},
        {7931, "The source must be a 3DS file"},
        {7932, "The destination must be an X file"},
        {8001, "Could not scan current directory"},
        {8002, "There are no more files in directory"},
        {8003, "Could not find path"},
        {8021, "Could not make file"},
        {8022, "Could not delete file"},
        {8023, "Could not copy file"},
        {8024, "Could not rename file"},
        {8025, "Could not move file"},
        {8101, "Could not create directory"},
        {8102, "Could not delete directory"},
        {8103, "Could not execute file"},
        {8201, "Could not open file for reading"},
        {8202, "Could not open file for writing"},
        {8203, "File is already open"},
        {8204, "File is not open"},
        {8211, "Cannot read from file"},
        {8212, "Cannot write to file"},
        {8213, "File number illegal"},
        {8301, "Could not connect to FTP"},
        {8302, "Cannot find FTP path"},
        {8303, "Cannot put FTP file"},
        {8304, "Cannot delete FTP file"},
        {8305, "Cannot get FTP file"},
        {8501, "Could not establish multiplayer connection"},
        {8502, "Could not find session"},
        {8503, "Could not setup session"},
        {8504, "Could not create net game"},
        {8505, "Could not join net game"},
        {8506, "Could not send net game message"},
        {8507, "Only 2 to 255 players can be specified"},
        {8508, "Player name required"},
        {8509, "Game name required"},
        {8510, "Connection number illegal"},
        {8511, "Session number illegal"},
        {8512, "Player number illegal"},
        {8513, "Not currently in net game session"},
        {8514, "Player number does not exist"},
        {8515, "Session number does not exist"},
        {8516, "Player could not be created"},
        {8517, "Player could not be deleted"},
        {8518, "Too many players in session"},
        {9001, "Checklist number illegal"},
        {9002, "Checklist number illegal"},
        {9003, "Checklist contains only numbers"},
        {9004, "Checklist contains only strings"},
        {9005, "Checklist does not exist"},
        {9011, "Could not find the device"},
        {9012, "Could not get texture memory"},
        {9013, "Could not get video memory"},
        {9014, "Could not get system memory"},
        {9701, "Could not load DLL"},
        {9702, "DLL does not exist"},
        {9703, "DLL already exists"},
        {9704, "Could not call DLL function"},
        {9705, "Index number illegal"}};

    if (runtimeError != 0)
    {
        std::string fullError;

        std::string runtimeErrorString = "(unknown)";
        auto it = runtimeCodeStrings.find(runtimeError);
        if (it != runtimeCodeStrings.end())
        {
            runtimeErrorString = it->second;
        }

        char runtimeErrorStorage[1024];
        wsprintf(runtimeErrorStorage, "Runtime Error %d - %s on line %d when calling '%s'.", runtimeError,
                 runtimeErrorString.c_str(), lineNumber, commandName);
        fullError += runtimeErrorStorage;

        // Is there any additional error info after EXEPATH string?
        DWORD dwEXEPathLength = strlen(globPtr->pEXEUnpackDirectory);
        const char* pSecretErrorMessage = globPtr->pEXEUnpackDirectory + dwEXEPathLength + 1;
        if (strlen(pSecretErrorMessage) > 0 && strlen(pSecretErrorMessage) < _MAX_PATH)
        {
            fullError += ".\n";
            fullError += pSecretErrorMessage;
        }

        char err[512];
        wsprintf(err, "OpenDarkBASIC (DBPro) has detected an unexpected issue and needs to restart your session.\n\n%s",
                 fullError.c_str());
        MessageBox(nullptr, err, "OpenDarkBASIC (DBPro) Problem Detected", MB_TOPMOST | MB_OK);
        ExitProcess(1);
    }
}

DLLEXPORT int closeEngine()
{
    auto coreDll = loadedPlugins["DBProCore.dll"];
    auto CloseDisplay = (DWORD(*)())GetProcAddress(coreDll, "?CloseDisplay@@YAKXZ");
    if (CloseDisplay() != 0)
    {
        std::cout << "Failed to close display." << std::endl;
        return 1;
    }

    return 0;
}

DLLEXPORT void exitProcess(int exitCode)
{
    ExitProcess(exitCode);
}
}