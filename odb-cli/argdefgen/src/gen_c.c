#include "argdefgen/action.h"
#include "argdefgen/templates.h"
#include "argdefgen/section.h"
#include "argdefgen/gen.h"
#include <string.h>

/* ------------------------------------------------------------------------- */
static void write_replace(FILE* fp, const char* src, const char* prefix)
{
    const char* ptr = src;
    while (ptr[0])
    {
        if (ptr[0] == '%')
        {
            if (strstr(ptr, "%prefix") == ptr)
            {
                ptr += sizeof("prefix");
                fprintf(fp, "%s", prefix);
            }
        }
        putc(*ptr++, fp);
    }
}

/* ------------------------------------------------------------------------- */
static void hdr_start_include_guard(FILE* fp, const char* prefix)
{
    fprintf(fp, "#ifndef ARGDEFGEN_%s_H\n", prefix);
    fprintf(fp, "#define ARGDEFGEN_%s_H\n\n", prefix);
}

/* ------------------------------------------------------------------------- */
static void hdr_end_include_guard(FILE* fp, const char* prefix)
{
    fprintf(fp, "#endif  /* ARGDEFGEN_%s_H */\n", prefix);
}

/* ------------------------------------------------------------------------- */
static void hdr_section_struct(FILE* fp, const char* prefix)
{
    write_replace(fp, adg_template_h_struct_section, prefix);
    putc('\n', fp);
}

/* ------------------------------------------------------------------------- */
static void hdr_action_struct(FILE* fp, const char* prefix)
{
    write_replace(fp, adg_template_h_struct_action, prefix);
    putc('\n', fp);
}

/* ------------------------------------------------------------------------- */
static void hdr_public_utils(FILE* fp, const char* prefix)
{
    write_replace(fp, adg_template_h_public_utils, prefix);
    putc('\n', fp);
}

/* ------------------------------------------------------------------------- */
static void src_public_utils(FILE* fp, const char* prefix)
{
    write_replace(fp, adg_template_c_public_utils, prefix);
    putc('\n', fp);
}

/* ------------------------------------------------------------------------- */
static void src_string_helpers_pre(FILE* fp, const char* prefix)
{
    write_replace(fp, adg_template_c_string_pre, prefix);
    putc('\n', fp);
}

/* ------------------------------------------------------------------------- */
static void src_string_helpers_post(FILE* fp, const char* prefix)
{
    write_replace(fp, adg_template_c_string_post, prefix);
    putc('\n', fp);
}

/* ------------------------------------------------------------------------- */
static void hdr_entry_function(FILE* fp, const char* prefix)
{
    fprintf(fp, "int parse_command_line(int argc, char** argv);\n\n");
}

/* ------------------------------------------------------------------------- */
static void src_section_table(FILE* fp, const char* prefix, struct adg_section** section_table)
{
    struct adg_section** section;
    fprintf(fp, "static const struct %ssection g_sections[] = {\n", prefix);
    for (section = section_table; *section; ++section)
    {
        fprintf(fp, "    {\"%s\", \"%s\"},\n", (*section)->name, (*section)->info);
    }
    fprintf(fp, "    {}\n};\n");

    fprintf(fp, "#define SECTION_VALID(section) \\\n");
    fprintf(fp, "    ((section)->name != NULL)\n\n");
}

/* ------------------------------------------------------------------------- */
static void
write_runafter_tables(struct adg_action** action_table, FILE* fp)
{
    struct adg_action** action;
    for (action = action_table; *action; ++action)
    {
        int* runafter;
        if ((*action)->runafter[0] == -1)
            continue;

        fprintf(fp, "static const int g_%s_runafter[] = {", (*action)->func_name);
        for (runafter = (*action)->runafter; *runafter != -1; ++runafter)
        {
            fprintf(fp, "%d, ", *runafter);
        }
        fprintf(fp, "-1};\n");
    }
}
static void
write_requires_tables(struct adg_action** action_table, FILE* fp)
{
    struct adg_action** action;
    for (action = action_table; *action; ++action)
    {
        int* requires;
        if ((*action)->requires[0] == -1)
            continue;

        fprintf(fp, "static const int g_%s_requires[] = {", (*action)->func_name);
        for (requires = (*action)->requires; *requires != -1; ++requires)
        {
            fprintf(fp, "%d, ", *requires);
        }
        fprintf(fp, "-1};\n");
    }
}
static void
write_metadeps_tables(struct adg_action** action_table, FILE* fp)
{
    struct adg_action** action;
    for (action = action_table; *action; ++action)
    {
        int* metadeps;
        if ((*action)->metadeps[0] == -1)
            continue;

        fprintf(fp, "static const int g_%s_metadeps[] = {", (*action)->func_name);
        for (metadeps = (*action)->metadeps; *metadeps != -1; ++metadeps)
        {
            fprintf(fp, "%d, ", *metadeps);
        }
        fprintf(fp, "-1};\n");
    }
}
void src_action_table(FILE* fp, const char* prefix, struct adg_action** action_table, struct adg_section** section_table)
{
    struct adg_action** actionp;
    int padding;
    int longopt_len = 0, argdoc_len = 0, func_len = 0, runafter_len = 0, requires_len = 0, metadeps_len = 0;
    for (actionp = action_table; *actionp; ++actionp)
    {
        int len = strlen((*actionp)->long_option);
        longopt_len = len > longopt_len ? len : longopt_len;
        len = strlen((*actionp)->arg_doc);
        argdoc_len = len > argdoc_len ? len : argdoc_len;
        len = strlen((*actionp)->func_name);
        func_len = len > func_len ? len : func_len;

        if ((*actionp)->runafter[0] != -1)
            runafter_len = len + sizeof("g__runafter") > runafter_len ? len + sizeof("g__runafter") : runafter_len;
        else
            runafter_len = sizeof("NULL") > runafter_len ? sizeof("NULL") : runafter_len;

        if ((*actionp)->requires[0] != -1)
            requires_len = len + sizeof("g__requires") > requires_len ? len + sizeof("g__requires") : requires_len;
        else
            requires_len = sizeof("NULL") > requires_len ? sizeof("NULL") : requires_len;

        if ((*actionp)->metadeps[0] != -1)
            metadeps_len = len + sizeof("g__metadeps") > metadeps_len ? len + sizeof("g__metadeps") : metadeps_len;
        else
            metadeps_len = sizeof("NULL") > metadeps_len ? sizeof("NULL") : metadeps_len;
    }

    write_runafter_tables(action_table, fp);
    write_requires_tables(action_table, fp);
    write_metadeps_tables(action_table, fp);

    fprintf(fp, "#define IMPLICIT 0x01\n");
    fprintf(fp, "#define META     0x02\n");

    fprintf(fp, "static const struct %saction g_actions[] = {\n", prefix);
    for (actionp = action_table; *actionp; ++actionp)
    {
        struct adg_action* action = *actionp;
        padding = longopt_len;
        fprintf(fp, "    { \"");
        padding -= fprintf(fp, "%s", action->long_option);
        fprintf(fp, "\", ");
        while (padding-- > 0)
            putc(' ', fp);

        padding = argdoc_len;
        fprintf(fp, "\"");
        padding -= fprintf(fp, "%s", action->arg_doc);
        fprintf(fp, "\", ");
        while (padding-- > 0)
            putc(' ', fp);

        padding = func_len;
        padding -= fprintf(fp, "%s", action->func_name);
        fprintf(fp, ", ");
        while (padding-- > 0)
            putc(' ', fp);

        padding = runafter_len;
        if ((*actionp)->runafter[0] != -1)
            padding -= fprintf(fp, "g_%s_runafter", action->func_name);
        else
            padding -= fprintf(fp, "NULL");
        fprintf(fp, ", ");
        while (padding-- > 0)
            putc(' ', fp);

        padding = requires_len;
        if ((*actionp)->requires[0] != -1)
            padding -= fprintf(fp, "g_%s_requires", action->func_name);
        else
            padding -= fprintf(fp, "NULL");
        fprintf(fp, ", ");
        while (padding-- > 0)
            putc(' ', fp);

        padding = metadeps_len;
        if ((*actionp)->metadeps[0] != -1)
            padding -= fprintf(fp, "g_%s_metadeps", action->func_name);
        else
            padding -= fprintf(fp, "NULL");
        fprintf(fp, ", ");
        while (padding-- > 0)
            putc(' ', fp);

        fprintf(fp, "{%2d, %3d}, ", action->arg_range.l, action->arg_range.h);
        fprintf(fp, "%2d, ", action->priority);

        fprintf(fp, "%d, ", adg_section_table_name_to_index(section_table, action->section_name));

        if (action->short_option != '\0')
            fprintf(fp, "'%c',", action->short_option);
        else
            fprintf(fp, " 0, ");

        fprintf(fp, "%d, ", (action->is_implicit ? 1 : 0 ) | (action->is_meta ? 2 : 0));
        fprintf(fp, "\"%s\"", action->help);

        fprintf(fp, " },\n");
    }
    fprintf(fp, "    {}  // sentinel\n");
    fprintf(fp, "};\n");

    fprintf(fp, "#define ACTION_VALID(action) \\\n");
    fprintf(fp, "    ((action)->full_option != NULL)\n\n");
}

/* ------------------------------------------------------------------------- */
void c_arg_parser_pre(FILE* fp, const char* prefix)
{
    write_replace(fp, adg_template_c_arg_parser_pre, prefix);
    putc('\n', fp);
}

/* ------------------------------------------------------------------------- */
void c_arg_parser_post(FILE* fp, const char* prefix)
{
    write_replace(fp, adg_template_c_arg_parser_post, prefix);
    putc('\n', fp);
}

/* ------------------------------------------------------------------------- */
void c_default_help_pre(FILE* fp, const char* prefix)
{
    write_replace(fp, adg_template_c_default_help_pre, prefix);
    putc('\n', fp);
}

/* ------------------------------------------------------------------------- */
void c_default_help_post(FILE* fp, const char* prefix)
{
    write_replace(fp, adg_template_c_default_help_post, prefix);
    putc('\n', fp);
}

/* ------------------------------------------------------------------------- */
struct adg_gen_interface adg_gen_c = {
    hdr_start_include_guard,
    hdr_end_include_guard,
    hdr_section_struct,
    hdr_action_struct,
    hdr_public_utils,
    src_public_utils,
    src_string_helpers_pre,
    src_string_helpers_post,
    hdr_entry_function,
    src_section_table,
    src_action_table,
    c_arg_parser_pre,
    c_arg_parser_post,
    c_default_help_pre,
    c_default_help_post
};
