#include "argdefgen/action.h"
#include "argdefgen/driver.h"
#include "argdefgen/gen.h"
#include "argdefgen/node.h"
#include "argdefgen/section.h"
#include <string.h>

static const char* argdef_file = NULL;
static const char* header_file = NULL;
static const char* source_file = NULL;
static const char* ast_file = NULL;
static const char* depgraph_file = NULL;

/* -------------------------------------------------------------------------- */
static void
print_help(const char* progname)
{
    fprintf(stdout, "Usage: %s [input file] [options...]\n", progname);
}

/* -------------------------------------------------------------------------- */
static int
parse_command_line(int argc, char** argv)
{
#define EXPECT_ARGUMENT                                                       \
    if (i + 1 >= argc)                                                        \
    {                                                                         \
        fprintf(stderr, "Error: Expected an argument after %s\n", argv[i]);   \
        return -1;                                                            \
    }
#define SET_IF_NULL(var, value)                                               \
    if (var != NULL)                                                          \
    {                                                                         \
        fprintf(stderr, "Error: Option \"%s\" used twice\n", argv[i]);        \
        return -1;                                                            \
    }                                                                         \
    var = value;

    int i;
    for (i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--source") == 0)
        {
            EXPECT_ARGUMENT
            SET_IF_NULL(source_file, argv[i+1]);
            i++;
            continue;
        }
        if (strcmp(argv[i], "--header") == 0)
        {
            EXPECT_ARGUMENT
            SET_IF_NULL(header_file, argv[i+1]);
            i++;
            continue;
        }
        if (strcmp(argv[i], "--ast") == 0)
        {
            EXPECT_ARGUMENT
            SET_IF_NULL(ast_file, argv[i+1]);
            i++;
            continue;
        }
        if (strcmp(argv[i], "--depgraph") == 0)
        {
            EXPECT_ARGUMENT
            SET_IF_NULL(depgraph_file, argv[i+1]);
            i++;
            continue;
        }
        if (strcmp(argv[i], "--help") == 0)
        {
            print_help(argv[0]);
            return -1;
        }

        if (argv[i][0] == '-')
        {
            fprintf(stderr, "Error: Invalid option \"%s\"", argv[i]);
            return -1;
        }

        if (argdef_file != NULL)
        {
            fprintf(stderr, "Error: Unexpected argument \"%s\"", argv[i]);
            return -1;
        }

        argdef_file = argv[i];
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
static union adg_node*
find_block(union adg_node* node, enum adg_node_type type)
{
    union adg_node* found;
    if (node->info.type == type)
        return node;

    if (node->base.left)
        if ((found = find_block(node->base.left, type)) != NULL)
            return found;
    if (node->base.right)
        if ((found = find_block(node->base.right, type)) != NULL)
            return found;

    return NULL;
}

/* ------------------------------------------------------------------------- */
static union adg_node*
parse_argdef_file_to_ast(void)
{
    FILE* fp;
    union adg_node* root;
    struct adg_driver driver;

    if (argdef_file)
    {
        fp = fopen(argdef_file, "r");
        if (fp == NULL)
        {
            fprintf(stderr, "Error: Failed to open file `%s'\n", argdef_file);
            goto open_argdef_file_failed;
        }
    }
    else
    {
        fp = stdin;
    }

    if (adg_driver_init(&driver) != 0)
        goto init_driver_failed;

    if ((root = adg_driver_parse_stream(&driver, fp)) == NULL)
        goto parse_file_failed;

    if (adg_node_generate_help_action_if_not_available(root) != 0)
        goto generate_help_node_failed;

    if (ast_file)
    {
        fprintf(stderr, "Exporting AST to `%s'\n", ast_file);
        adg_node_export_dot(root, ast_file);
    }

    adg_driver_deinit(&driver);
    if (argdef_file)
        fclose(fp);
    return root;

    return root;

    generate_help_node_failed :
    parse_file_failed         : adg_driver_deinit(&driver);
    init_driver_failed        : if (argdef_file) fclose(fp);
    open_argdef_file_failed   : return NULL;
}

/* -------------------------------------------------------------------------- */
int main(int argc, char** argv)
{
    struct adg_action** action_table;
    struct adg_section** section_table;
    union adg_node* root;
    union adg_node* block;

    if (parse_command_line(argc, argv) != 0)
        goto parse_command_line_failed;

    root = parse_argdef_file_to_ast();
    if (root == NULL)
        goto parse_argdef_file_failed;

    block = find_block(root, ADG_ACTION_TABLE);
    if (block == NULL)
    {
        fprintf(stderr, "Warning: Missing action table block in input file\n");
        action_table = adg_action_table_new_empty();
        if (action_table == NULL)
            goto create_action_table_failed;
        section_table = adg_section_table_new_empty();
        if (section_table == NULL)
            goto create_section_table_failed;
    }
    else
    {
        action_table = adg_action_table_from_nodes(block->action_table.sections);
        if (action_table == NULL)
            goto create_action_table_failed;

        section_table = adg_section_table_from_nodes(block->action_table.sections);
        if (section_table == NULL)
            goto create_section_table_failed;
    }

    if (depgraph_file)
    {
        fprintf(stderr, "Exporting depgraph to `%s'\n", depgraph_file);
        adg_action_table_export_dot(action_table, depgraph_file);
    }

    /* Write header if applicable */
    block = find_block(root, ADG_HEADER_PREAMBLE);
    if (header_file)
    {
        FILE* fp = fopen(header_file, "w");
        if (fp == NULL)
        {
            fprintf(stderr, "Error: Failed to open file `%s'\n", header_file);
            goto export_failed;
        }

        fprintf(fp, "#pragma once\n\n");

        block = find_block(root, ADG_HEADER_PREAMBLE);
        if (block)
            fprintf(fp, "%s", block->source_preamble.text);

        adg_gen_cpp_write_typedefs(fp);
        adg_gen_cpp_write_entry_function_forward_decl(fp);

        block = find_block(root, ADG_HEADER_POSTAMBLE);
        if (block)
            fprintf(fp, "%s", block->source_postamble.text);

        fclose(fp);
    }
    else
    {
        block = find_block(root, ADG_HEADER_PREAMBLE);
        if (block)
            fprintf(stderr, "Warning: argdef contains a header preamble block, but no --header argument was specified. Cannot export.\n");
        block = find_block(root, ADG_HEADER_POSTAMBLE);
        if (block)
            fprintf(stderr, "Warning: argdef contains a header preamble block, but no --header argument was specified. Cannot export.\n");
    }

    /* Write source */
    {
        FILE* fp;

        if (source_file)
        {
            fp = fopen(source_file, "w");
            if (fp == NULL)
            {
                fprintf(stderr, "Error: Failed to open file `%s'\n", source_file);
                goto export_failed;
            }
        }
        else
        {
            fp = stdout;
        }

        if (header_file == NULL)
            adg_gen_cpp_write_typedefs(fp);

        adg_gen_cpp_write_helpers_forward_decl(fp);

        block = find_block(root, ADG_SOURCE_PREAMBLE);
        if (block)
            fprintf(fp, "%s", block->source_preamble.text);

        adg_gen_cpp_write_argparse_preamble(fp);
        adg_gen_cpp_write_section_struct_def(fp);
        adg_gen_cpp_write_action_struct_def(fp);
        adg_gen_cpp_write_section_table(section_table, fp);
        adg_gen_cpp_write_action_table(action_table, section_table, fp);
        adg_gen_cpp_write_argparse_postamble(fp);
        adg_gen_cpp_write_helpers_impl(fp);

        block = find_block(root, ADG_SOURCE_POSTAMBLE);
        if (block)
            fprintf(fp, "%s", block->source_postamble.text);

        if (source_file)
            fclose(fp);
    }

    adg_section_table_destroy(section_table);
    adg_action_table_destroy(action_table);
    adg_node_destroy_recursive(root);

    return 0;

    export_failed               : adg_section_table_destroy(section_table);
    create_section_table_failed : adg_action_table_destroy(action_table);
    create_action_table_failed  : adg_node_destroy_recursive(root);
    parse_argdef_file_failed    :
    parse_command_line_failed   : return -1;
}
