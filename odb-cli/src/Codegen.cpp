#include "odb-cli/AST.hpp"
#include "odb-cli/Codegen.hpp"
#include "odb-cli/Commands.hpp"
#include "odb-cli/SDK.hpp"

extern "C" {
#include "odb-compiler/codegen/ir.h"
#include "odb-compiler/codegen/target.h"
#include "odb-compiler/link/link.h"
#include "odb-compiler/sdk/used_cmds.h"
#include "odb-util/fs.h"
#include "odb-util/log.h"
#include "odb-util/process.h"
}

static std::string      outputExe_;
static bool             dumpIR_ = false;
static enum target_arch arch_ = TARGET_x86_64;
#if defined(ODBSDK_PLATFORM_LINUX)
static enum target_platform platform_ = TARGET_LINUX;
#else
static enum target_platform platform_ = TARGET_WINDOWS;
#endif

// ----------------------------------------------------------------------------
bool
setArch(const std::vector<std::string>& args)
{
    if (args[0] == "i386")
        arch_ = TARGET_i386;
    else if (args[0] == "x86_64")
        arch_ = TARGET_x86_64;
    else if (args[0] == "aarch64")
        arch_ = TARGET_AArch64;
    else
        log_codegen_err("Unknown architecture {quote:%s}\n", args[0].c_str());

    return true;
}

// ----------------------------------------------------------------------------
bool
setPlatform(const std::vector<std::string>& args)
{
    if (args[0] == "windows")
        platform_ = TARGET_WINDOWS;
    else if (args[0] == "macos")
        platform_ = TARGET_MACOS;
    else if (args[0] == "linux")
        platform_ = TARGET_LINUX;
    else
        log_codegen_err("Unknown platform {quote:%s}\n", args[0].c_str());

    return true;
}

// ----------------------------------------------------------------------------
bool
dumpIR(const std::vector<std::string>& args)
{
    dumpIR_ = true;
    return true;
}

// ----------------------------------------------------------------------------
static int
set_path_to_arch_platform_dir(struct ospath* path)
{
    fs_get_path_to_self(path);
    ospath_dirname(path);
    ospath_dirname(path);
    ospath_dirname(path);
    ospath_dirname(path);
    ospath_join_cstr(path, target_arch_to_name(getTargetArch()));
    ospath_join_cstr(path, target_platform_to_name(getTargetPlatform()));
    return 0;
}

bool
output(const std::vector<std::string>& args)
{
    if (getSDKType() == SDK_DBPRO)
    {
        platform_ = TARGET_WINDOWS;
        arch_ = TARGET_i386;
    }

    log_info("[codegen] ", "Compiling {emph:%s}\n", getSourceFilepath());
    outputExe_ = args[0];

    /* Path to the compiler's architecture/platform directory, e.g. i386/windows/ */
    struct ospath apdir = empty_ospath();
    set_path_to_arch_platform_dir(&apdir);
    log_dbg("[codegen] ", "apdir: {quote:%s}\n", ospath_cstr(apdir));

    /* Location for intermediate files such as object files */
    struct ospath tmpdir = empty_ospath();
    ospath_set_cstr(&tmpdir, outputExe_.c_str());
    ospath_dirname(&tmpdir);
    ospath_join_cstr(&tmpdir, "_odbtmp");
    // TODO
    //if (fs_dir_exists(ospathc(tmpdir)))
    //    fs_remove_directory(ospathc(tmpdir));
    fs_make_dir(ospathc(tmpdir));
    log_dbg("[codegen] ", "tmpdir: {quote:%s}\n", ospath_cstr(tmpdir));

    /* Directory where the compiled executable is written to */
    struct ospath outdir = empty_ospath();
    ospath_set_cstr(&outdir, outputExe_.c_str());
    ospath_dirname(&outdir);
    log_dbg("[codegen] ", "outdir: {quote:%s}\n", ospath_cstr(outdir));

    /* Create a list of commands that were actually used, which get loaded by
     * the harness */
    struct used_cmds_hm* used_cmds_hm;
    used_cmds_init(&used_cmds_hm);
    used_cmds_append(&used_cmds_hm, getAST());
    struct cmd_ids* used_cmds_list = used_cmds_finalize(used_cmds_hm);

    /* Harness needs to know the module name of the main DBA */
    struct ospath maindbaname = empty_ospath();
    ospath_set_cstr(&maindbaname, getSourceFilepath());
    ospath_filename(&maindbaname);
    ospath_remove_ext(&maindbaname);
    log_dbg("[codegen] ", "maindbaname: {quote:%s}\n", ospath_cstr(maindbaname));
    
    log_info("[codegen] ", "Generating harness\n");
    struct ospath harnessobj = empty_ospath();
    ospath_set(&harnessobj, ospathc(tmpdir));
    ospath_join_cstr(&harnessobj, "odbharness.o");
    struct ir_module* ir = ir_alloc("odbharness");
    ir_create_harness(
        ir,
        getPluginList(),
        getCommandList(),
        used_cmds_list,
        ospath_cstr(maindbaname),
        getSDKType(),
        arch_,
        platform_);
    ir_compile(ir, ospath_cstr(harnessobj), arch_, platform_);
    if (dumpIR_)
        ir_dump(ir);
    ir_free(ir);

    cmd_ids_deinit(used_cmds_list);

    struct ospath objfilepath = empty_ospath();
    struct ospathc srcfilename = cstr_ospathc(getSourceFilepath());
    ospathc_filename(&srcfilename);
    ospath_set(&objfilepath, ospathc(tmpdir));
    ospath_join(&objfilepath, srcfilename);
    utf8_append_cstr(&objfilepath.str, ".o");
    ir = ir_alloc(ospath_cstr(maindbaname));
    ir_translate_ast(
        ir,
        getAST(),
        getSDKType(),
        getCommandList(),
        getSourceFilepath(),
        getSource());
    ir_optimize(ir);
    ir_compile(ir, ospath_cstr(objfilepath), arch_, platform_);
    if (dumpIR_)
        ir_dump(ir);
    ir_free(ir);

    struct ospath odbrt = empty_ospath();
    if (getSDKType() == SDK_ODB)
    {
        ospath_set(&odbrt, ospathc(apdir));
        switch (platform_)
        {
            case TARGET_WINDOWS: ospath_join_cstr(&odbrt, "odb-sdk/runtime/odb-runtime.lib"); break;
            case TARGET_LINUX: ospath_join_cstr(&odbrt, "odb-sdk/runtime/odb-runtime.so"); break;
            case TARGET_MACOS: ospath_join_cstr(&odbrt, "odb-sdk/runtime/odb-runtime.dylib"); break;
        }
    }

    struct ospath kernel32 = empty_ospath();
    if (platform_ == TARGET_WINDOWS)
    {
        ospath_set(&kernel32, ospathc(apdir));
        ospath_join_cstr(&kernel32, "lib/kernel32.lib");
    }
    
    log_info("[link] ", "Linking {emph:%s}\n", outputExe_.c_str());
    const char* objfiles[] = {ospath_cstr(objfilepath),ospath_cstr(harnessobj), ospath_cstr(odbrt), ospath_cstr(kernel32)};
    odb_link(objfiles, 4, outputExe_.c_str(), arch_, platform_);

    ospath_dirname(&odbrt);
    switch (platform_)
    {
        case TARGET_WINDOWS:
            ospath_join_cstr(&odbrt, "odb-runtime.dll");
            ospath_join_cstr(&outdir, "odb-runtime.dll");
            break;
        case TARGET_LINUX:
            ospath_join_cstr(&odbrt, "odb-runtime.so");
            ospath_join_cstr(&outdir, "odb-runtime.so");
            break;
        case TARGET_MACOS:
            ospath_join_cstr(&odbrt, "odb-runtime.dylib");
            ospath_join_cstr(&outdir, "odb-runtime.dylib");
            break;
    }
    fs_copy_file_if_different(ospathc(odbrt), ospathc(outdir));
    ospath_dirname(&outdir);
    
    ospath_set(&odbrt, ospathc(apdir));
    switch (platform_)
    {
        case TARGET_WINDOWS:
            ospath_join_cstr(&odbrt, "bin/odb-util.dll");
            ospath_join_cstr(&outdir, "odb-util.dll");
            break;
        case TARGET_LINUX:
            ospath_join_cstr(&odbrt, "bin/odb-util.so");
            ospath_join_cstr(&outdir, "odb-util.so");
            break;
        case TARGET_MACOS:
            ospath_join_cstr(&odbrt, "bin/odb-util.dylib");
            ospath_join_cstr(&outdir, "odb-util.dylib");
            break;
    }
    fs_copy_file_if_different(ospathc(odbrt), ospathc(outdir));
    ospath_dirname(&outdir);
    
    // TODO
    //fs_remove_directory(ospathc(tmpdir));

    ospath_deinit(kernel32);
    ospath_deinit(odbrt);
    ospath_deinit(objfilepath);
    ospath_deinit(harnessobj);
    ospath_deinit(maindbaname);
    ospath_deinit(outdir);
    ospath_deinit(tmpdir);
    ospath_deinit(apdir);

    return true;
}

// ----------------------------------------------------------------------------
bool exec_output(const std::vector<std::string>& args)
{
    const char* argv[] = {outputExe_.c_str(), NULL};
    struct ospath working_dir = empty_ospath();
    struct utf8 out = empty_utf8();
    ospath_set_cstr(&working_dir, outputExe_.c_str());
    ospath_dirname(&working_dir);
    log_info("[exec] ", "Executing file {quote:%s}\n", outputExe_.c_str());
    int result = process_run(
        cstr_ospathc(outputExe_.c_str()),
        ospathc(working_dir),
        argv,
        empty_utf8_view(), &out, NULL, 0);
    if (out.data[out.len-1] != '\n')
        utf8_append_cstr(&out, "\n");
    log_raw("%s", utf8_cstr(out));
    log_info("[exec] ", "Process exited with %d\n", result);
    ospath_deinit(working_dir);
    utf8_deinit(out);
    return result == 0;
}

// ----------------------------------------------------------------------------
enum target_arch
getTargetArch(void)
{
    return arch_;
}

// ----------------------------------------------------------------------------
enum target_platform
getTargetPlatform(void)
{
    return platform_;
}
