#include "odb-compiler/build_info.h"
#include "odb-util/init.h"
#include "odb-util/log.h"

/* clang-format off */
int list_warnings(int argc, char** argv) { return -1; }
int generate_output(int argc, char** argv) { return -1; }
int dump_ir(int argc, char** argv) { return -1; }
int set_target_platform(int argc, char** argv) { return -1; }
int set_target_arch(int argc, char** argv) { return -1; }
int generate_ir(int argc, char** argv) { return -1; }
int dump_ast2(int argc, char** argv) { return -1; }
int dump_ast1(int argc, char** argv) { return -1; }
int set_dbpro(int argc, char** argv) { return -1; }
int set_dba(int argc, char** argv) { return -1; }
int set_input(int argc, char** argv) { return -1; }
int do_semantic_checks(int argc, char** argv) { return -1; }
int parse_input(int argc, char** argv) { return -1; }
int dump_commands(int argc, char** argv) { return -1; }
int print_sdk(int argc, char** argv) { return -1; }
int set_sdk_plugins(int argc, char** argv) { return -1; }
int set_sdk_type(int argc, char** argv) { return -1; }
int set_sdk_root(int argc, char** argv) { return -1; }
int load_commands(int argc, char** argv) { return -1; }
int setup_sdk(int argc, char** argv) { return -1; }
int configure_warning(int argc, char** argv) { return -1; }
int print_help(int argc, char** argv) { return -1; }
int print_commit_hash(int argc, char** argv)
{
    log_raw("%s\n", build_info_commit_hash());
    return 0;
}
int print_version(int argc, char** argv) {
    log_raw("%s\n", build_info_version());
    return 0;
}
/* clang-format on */

#include "odb-cli/args.cli.h"

// ----------------------------------------------------------------------------
int
main(int argc, char** argv)
{
    if (odbutil_init() != 0)
        goto odbsdk_init_failed;

    return odbutil_deinit();

odbsdk_init_failed:
    return -1;
}
