; ModuleID = 'input.dba'
source_filename = "input.dba"

@0 = private global [11 x i8] c"hello world"
@1 = private unnamed_addr constant [14 x i8] c"DBProCore.dll\00", align 1
@2 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@3 = private unnamed_addr constant [20 x i8] c"DBProImageDebug.dll\00", align 1
@4 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@5 = private unnamed_addr constant [20 x i8] c"DBProSetupDebug.dll\00", align 1
@6 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@7 = private unnamed_addr constant [22 x i8] c"DBProVectorsDebug.dll\00", align 1
@8 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@9 = private unnamed_addr constant [22 x i8] c"DBProBasic2DDebug.dll\00", align 1
@10 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@11 = private unnamed_addr constant [20 x i8] c"DBProMusicDebug.dll\00", align 1
@12 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@13 = private unnamed_addr constant [29 x i8] c"DBProAdvancedMatrixDebug.dll\00", align 1
@14 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@15 = private unnamed_addr constant [22 x i8] c"DBProSpritesDebug.dll\00", align 1
@16 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@17 = private unnamed_addr constant [24 x i8] c"DBProAnimationDebug.dll\00", align 1
@18 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@19 = private unnamed_addr constant [18 x i8] c"DBProFTPDebug.dll\00", align 1
@20 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@21 = private unnamed_addr constant [24 x i8] c"DBProParticlesDebug.dll\00", align 1
@22 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@23 = private unnamed_addr constant [24 x i8] c"DBProMemblocksDebug.dll\00", align 1
@24 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@25 = private unnamed_addr constant [19 x i8] c"DBProTextDebug.dll\00", align 1
@26 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@27 = private unnamed_addr constant [21 x i8] c"DBProBitmapDebug.dll\00", align 1
@28 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@29 = private unnamed_addr constant [21 x i8] c"DBProCameraDebug.dll\00", align 1
@30 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@31 = private unnamed_addr constant [22 x i8] c"DBProBasic3DDebug.dll\00", align 1
@32 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@33 = private unnamed_addr constant [19 x i8] c"DBProFileDebug.dll\00", align 1
@34 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@35 = private unnamed_addr constant [30 x i8] c"DBProMultiplayerPlusDebug.dll\00", align 1
@36 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@37 = private unnamed_addr constant [20 x i8] c"DBProInputDebug.dll\00", align 1
@38 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@39 = private unnamed_addr constant [20 x i8] c"DBProLightDebug.dll\00", align 1
@40 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@41 = private unnamed_addr constant [21 x i8] c"DBProMatrixDebug.dll\00", align 1
@42 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@43 = private unnamed_addr constant [26 x i8] c"DBProMultiplayerDebug.dll\00", align 1
@44 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@45 = private unnamed_addr constant [20 x i8] c"DBProQ3BSPDebug.dll\00", align 1
@46 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@47 = private unnamed_addr constant [20 x i8] c"DBProSoundDebug.dll\00", align 1
@48 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@49 = private unnamed_addr constant [21 x i8] c"DBProSystemDebug.dll\00", align 1
@50 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@51 = private unnamed_addr constant [25 x i8] c"DBProTransformsDebug.dll\00", align 1
@52 = private unnamed_addr constant [16 x i8] c"Loading plugin \00", align 1
@53 = private unnamed_addr constant [45 x i8] c"Failed to load plugin. GetLastError returned\00", align 1

define void @db_main() {
entry:
  ret void
}

declare dllimport void @"?PrintS@@YAXPAD@Z"(i8*)

declare dllimport x86_stdcallcc i8* @LoadLibraryA(i8*)

declare dllimport x86_stdcallcc i64 @GetLastError()

declare dllimport x86_stdcallcc i32 ()* @GetProcAddress(i8*, i8*)

define i32 @main() {
load_DBProCore:
  %DBProCore_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @1, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @2, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @1, i32 0, i32 0))
  %0 = icmp ne i8* %DBProCore_hmodule, null
  br i1 %0, label %load_DBProImageDebug, label %failed_to_load_plugins

load_DBProImageDebug:                             ; preds = %load_DBProCore
  %DBProImageDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @3, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @4, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @3, i32 0, i32 0))
  %1 = icmp ne i8* %DBProImageDebug_hmodule, null
  br i1 %1, label %load_DBProSetupDebug, label %failed_to_load_plugins

load_DBProSetupDebug:                             ; preds = %load_DBProImageDebug
  %DBProSetupDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @5, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @6, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @5, i32 0, i32 0))
  %2 = icmp ne i8* %DBProSetupDebug_hmodule, null
  br i1 %2, label %load_DBProVectorsDebug, label %failed_to_load_plugins

load_DBProVectorsDebug:                           ; preds = %load_DBProSetupDebug
  %DBProVectorsDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @7, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @8, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @7, i32 0, i32 0))
  %3 = icmp ne i8* %DBProVectorsDebug_hmodule, null
  br i1 %3, label %load_DBProBasic2DDebug, label %failed_to_load_plugins

load_DBProBasic2DDebug:                           ; preds = %load_DBProVectorsDebug
  %DBProBasic2DDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @9, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @10, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @9, i32 0, i32 0))
  %4 = icmp ne i8* %DBProBasic2DDebug_hmodule, null
  br i1 %4, label %load_DBProMusicDebug, label %failed_to_load_plugins

load_DBProMusicDebug:                             ; preds = %load_DBProBasic2DDebug
  %DBProMusicDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @11, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @12, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @11, i32 0, i32 0))
  %5 = icmp ne i8* %DBProMusicDebug_hmodule, null
  br i1 %5, label %load_DBProAdvancedMatrixDebug, label %failed_to_load_plugins

load_DBProAdvancedMatrixDebug:                    ; preds = %load_DBProMusicDebug
  %DBProAdvancedMatrixDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([29 x i8], [29 x i8]* @13, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @14, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([29 x i8], [29 x i8]* @13, i32 0, i32 0))
  %6 = icmp ne i8* %DBProAdvancedMatrixDebug_hmodule, null
  br i1 %6, label %load_DBProSpritesDebug, label %failed_to_load_plugins

load_DBProSpritesDebug:                           ; preds = %load_DBProAdvancedMatrixDebug
  %DBProSpritesDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @15, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @16, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @15, i32 0, i32 0))
  %7 = icmp ne i8* %DBProSpritesDebug_hmodule, null
  br i1 %7, label %load_DBProAnimationDebug, label %failed_to_load_plugins

load_DBProAnimationDebug:                         ; preds = %load_DBProSpritesDebug
  %DBProAnimationDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([24 x i8], [24 x i8]* @17, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @18, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([24 x i8], [24 x i8]* @17, i32 0, i32 0))
  %8 = icmp ne i8* %DBProAnimationDebug_hmodule, null
  br i1 %8, label %load_DBProFTPDebug, label %failed_to_load_plugins

load_DBProFTPDebug:                               ; preds = %load_DBProAnimationDebug
  %DBProFTPDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @19, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @20, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @19, i32 0, i32 0))
  %9 = icmp ne i8* %DBProFTPDebug_hmodule, null
  br i1 %9, label %load_DBProParticlesDebug, label %failed_to_load_plugins

load_DBProParticlesDebug:                         ; preds = %load_DBProFTPDebug
  %DBProParticlesDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([24 x i8], [24 x i8]* @21, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @22, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([24 x i8], [24 x i8]* @21, i32 0, i32 0))
  %10 = icmp ne i8* %DBProParticlesDebug_hmodule, null
  br i1 %10, label %load_DBProMemblocksDebug, label %failed_to_load_plugins

load_DBProMemblocksDebug:                         ; preds = %load_DBProParticlesDebug
  %DBProMemblocksDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([24 x i8], [24 x i8]* @23, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @24, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([24 x i8], [24 x i8]* @23, i32 0, i32 0))
  %11 = icmp ne i8* %DBProMemblocksDebug_hmodule, null
  br i1 %11, label %load_DBProTextDebug, label %failed_to_load_plugins

load_DBProTextDebug:                              ; preds = %load_DBProMemblocksDebug
  %DBProTextDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @25, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @26, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @25, i32 0, i32 0))
  %12 = icmp ne i8* %DBProTextDebug_hmodule, null
  br i1 %12, label %load_DBProBitmapDebug, label %failed_to_load_plugins

load_DBProBitmapDebug:                            ; preds = %load_DBProTextDebug
  %DBProBitmapDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @27, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @28, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @27, i32 0, i32 0))
  %13 = icmp ne i8* %DBProBitmapDebug_hmodule, null
  br i1 %13, label %load_DBProCameraDebug, label %failed_to_load_plugins

load_DBProCameraDebug:                            ; preds = %load_DBProBitmapDebug
  %DBProCameraDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @29, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @30, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @29, i32 0, i32 0))
  %14 = icmp ne i8* %DBProCameraDebug_hmodule, null
  br i1 %14, label %load_DBProBasic3DDebug, label %failed_to_load_plugins

load_DBProBasic3DDebug:                           ; preds = %load_DBProCameraDebug
  %DBProBasic3DDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @31, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @32, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @31, i32 0, i32 0))
  %15 = icmp ne i8* %DBProBasic3DDebug_hmodule, null
  br i1 %15, label %load_DBProFileDebug, label %failed_to_load_plugins

load_DBProFileDebug:                              ; preds = %load_DBProBasic3DDebug
  %DBProFileDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @33, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @34, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @33, i32 0, i32 0))
  %16 = icmp ne i8* %DBProFileDebug_hmodule, null
  br i1 %16, label %load_DBProMultiplayerPlusDebug, label %failed_to_load_plugins

load_DBProMultiplayerPlusDebug:                   ; preds = %load_DBProFileDebug
  %DBProMultiplayerPlusDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([30 x i8], [30 x i8]* @35, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @36, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([30 x i8], [30 x i8]* @35, i32 0, i32 0))
  %17 = icmp ne i8* %DBProMultiplayerPlusDebug_hmodule, null
  br i1 %17, label %load_DBProInputDebug, label %failed_to_load_plugins

load_DBProInputDebug:                             ; preds = %load_DBProMultiplayerPlusDebug
  %DBProInputDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @37, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @38, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @37, i32 0, i32 0))
  %18 = icmp ne i8* %DBProInputDebug_hmodule, null
  br i1 %18, label %load_DBProLightDebug, label %failed_to_load_plugins

load_DBProLightDebug:                             ; preds = %load_DBProInputDebug
  %DBProLightDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @39, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @40, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @39, i32 0, i32 0))
  %19 = icmp ne i8* %DBProLightDebug_hmodule, null
  br i1 %19, label %load_DBProMatrixDebug, label %failed_to_load_plugins

load_DBProMatrixDebug:                            ; preds = %load_DBProLightDebug
  %DBProMatrixDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @41, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @42, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @41, i32 0, i32 0))
  %20 = icmp ne i8* %DBProMatrixDebug_hmodule, null
  br i1 %20, label %load_DBProMultiplayerDebug, label %failed_to_load_plugins

load_DBProMultiplayerDebug:                       ; preds = %load_DBProMatrixDebug
  %DBProMultiplayerDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @43, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @44, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @43, i32 0, i32 0))
  %21 = icmp ne i8* %DBProMultiplayerDebug_hmodule, null
  br i1 %21, label %load_DBProQ3BSPDebug, label %failed_to_load_plugins

load_DBProQ3BSPDebug:                             ; preds = %load_DBProMultiplayerDebug
  %DBProQ3BSPDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @45, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @46, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @45, i32 0, i32 0))
  %22 = icmp ne i8* %DBProQ3BSPDebug_hmodule, null
  br i1 %22, label %load_DBProSoundDebug, label %failed_to_load_plugins

load_DBProSoundDebug:                             ; preds = %load_DBProQ3BSPDebug
  %DBProSoundDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @47, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @48, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @47, i32 0, i32 0))
  %23 = icmp ne i8* %DBProSoundDebug_hmodule, null
  br i1 %23, label %load_DBProSystemDebug, label %failed_to_load_plugins

load_DBProSystemDebug:                            ; preds = %load_DBProSoundDebug
  %DBProSystemDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @49, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @50, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @49, i32 0, i32 0))
  %24 = icmp ne i8* %DBProSystemDebug_hmodule, null
  br i1 %24, label %load_DBProTransformsDebug, label %failed_to_load_plugins

load_DBProTransformsDebug:                        ; preds = %load_DBProSystemDebug
  %DBProTransformsDebug_hmodule = call i8* @LoadLibraryA(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @51, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @52, i32 0, i32 0))
  call void @puts(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @51, i32 0, i32 0))
  %25 = icmp ne i8* %DBProTransformsDebug_hmodule, null
  br i1 %25, label %launch_game, label %failed_to_load_plugins
  br label %launch_game

failed_to_load_plugins:                           ; preds = %load_DBProTransformsDebug, %load_DBProSystemDebug, %load_DBProSoundDebug, %load_DBProQ3BSPDebug, %load_DBProMultiplayerDebug, %load_DBProMatrixDebug, %load_DBProLightDebug, %load_DBProInputDebug, %load_DBProMultiplayerPlusDebug, %load_DBProFileDebug, %load_DBProBasic3DDebug, %load_DBProCameraDebug, %load_DBProBitmapDebug, %load_DBProTextDebug, %load_DBProMemblocksDebug, %load_DBProParticlesDebug, %load_DBProFTPDebug, %load_DBProAnimationDebug, %load_DBProSpritesDebug, %load_DBProAdvancedMatrixDebug, %load_DBProMusicDebug, %load_DBProBasic2DDebug, %load_DBProVectorsDebug, %load_DBProSetupDebug, %load_DBProImageDebug, %load_DBProCore
  call void @puts(i8* getelementptr inbounds ([45 x i8], [45 x i8]* @53, i32 0, i32 0))
  %27 = alloca i8, i32 100, align 4
  %28 = call i64 @GetLastError()
  %29 = call i8* @_ltoa(i64 %28, i8* %27, i32 10)
  call void @puts(i8* %27)
  ret i32 1

launch_game:                                      ; preds = %load_DBProTransformsDebug, %load_DBProTransformsDebug
  call void @db_main()
  ret i32 0
}

declare void @puts(i8*)

declare i8* @_ltoa(i64, i8*, i32)
