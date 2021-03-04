#include "argdefgen/action.h"
#include "argdefgen/argparse.h"
#include "argdefgen/gen.h"
#include "argdefgen/section.h"
#include <string.h>

/* ------------------------------------------------------------------------- */
void
adg_gen_cpp_write_typedefs(FILE* fp)
{
    fprintf(fp, "#include <string>\n");
    fprintf(fp, "#include <vector>\n\n");

    fprintf(fp, "typedef std::vector<std::string> ArgList;\n");
    fprintf(fp, "typedef bool (*Handler)(const ArgList& args);\n\n");

    fprintf(fp, "struct Action;\n");
    fprintf(fp, "struct ActionHandler\n{\n");
    fprintf(fp, "    static ActionHandler fromShortOption(char shortOption, const std::vector<std::string>& args={});\n");
    fprintf(fp, "    static ActionHandler fromFullOption(const std::string& fullOption, const std::vector<std::string>& args={});\n");
    fprintf(fp, "    static ActionHandler fromId(int actionId, const std::vector<std::string>& args={});\n");

    fprintf(fp, "    ArgList args;\n");
    fprintf(fp, "    int actionId = -1;\n");
    fprintf(fp, "};\n\n");

    fprintf(fp, "typedef ActionHandler (*MetaHandler)(const ArgList& args);\n\n");
}

/* ------------------------------------------------------------------------- */
void
adg_gen_cpp_write_entry_function_forward_decl(FILE* fp)
{
    fprintf(fp, "bool parseCommandLine(int argc, char** argv);\n\n");
}

/* ------------------------------------------------------------------------- */
void
adg_gen_cpp_write_action_struct_def(FILE* fp)
{
    fprintf(fp, "struct Action\n{\n");
    fprintf(fp, "    const char* fullOption;\n");
    fprintf(fp, "    const char* argDoc;\n");
    fprintf(fp, "    const union HU {\n");
    fprintf(fp, "        HU();\n");
    fprintf(fp, "        HU(Handler standard);\n");
    fprintf(fp, "        HU(MetaHandler meta);\n");
    fprintf(fp, "        Handler standard;\n");
    fprintf(fp, "        MetaHandler meta;\n");
    fprintf(fp, "    } handler;\n");
    fprintf(fp, "    const int* runafter;\n");
    fprintf(fp, "    const int* requires;\n");
    fprintf(fp, "    const int* metadeps;\n");
    fprintf(fp, "    const struct { int l; int h; } argRange;\n");
    fprintf(fp, "    const int priority;\n");
    fprintf(fp, "    const int sectionId;\n");
    fprintf(fp, "    const char shortOption;\n");
    fprintf(fp, "    const unsigned char type;\n");
    fprintf(fp, "    const char* help;\n");
    fprintf(fp, "};\n");
}

/* ------------------------------------------------------------------------- */
void
adg_gen_cpp_write_section_struct_def(FILE* fp)
{
    fprintf(fp, "struct Section\n{\n");
    fprintf(fp, "    const char* name;\n");
    fprintf(fp, "    const char* info;\n");
    fprintf(fp, "};\n");
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

        fprintf(fp, "static const int %s_runafter_[] = {", (*action)->func_name);
        for (runafter = (*action)->runafter; *runafter != -1; ++runafter)
        {
            fprintf(fp, "%d, ", *runafter);
        }
        fprintf(fp, "-1};\n");
    }
}

/* ------------------------------------------------------------------------- */
static void
write_requires_tables(struct adg_action** action_table, FILE* fp)
{
    struct adg_action** action;
    for (action = action_table; *action; ++action)
    {
        int* requires;
        if ((*action)->requires[0] == -1)
            continue;

        fprintf(fp, "static const int %s_requires_[] = {", (*action)->func_name);
        for (requires = (*action)->requires; *requires != -1; ++requires)
        {
            fprintf(fp, "%d, ", *requires);
        }
        fprintf(fp, "-1};\n");
    }
}

/* ------------------------------------------------------------------------- */
static void
write_metadeps_tables(struct adg_action** action_table, FILE* fp)
{
    struct adg_action** action;
    for (action = action_table; *action; ++action)
    {
        int* metadeps;
        if ((*action)->metadeps[0] == -1)
            continue;

        fprintf(fp, "static const int %s_metadeps_[] = {", (*action)->func_name);
        for (metadeps = (*action)->metadeps; *metadeps != -1; ++metadeps)
        {
            fprintf(fp, "%d, ", *metadeps);
        }
        fprintf(fp, "-1};\n");
    }
}

/* ------------------------------------------------------------------------- */
void
adg_gen_cpp_write_section_table(struct adg_section** section_table, FILE* fp)
{
    struct adg_section** section;
    fprintf(fp, "static const Section sections_[] = {\n");
    for (section = section_table; *section; ++section)
    {
        fprintf(fp, "    {\"%s\", \"%s\"},\n", (*section)->name, (*section)->info);
    }
    fprintf(fp, "    {}\n};\n");

    fprintf(fp, "#define SECTION_VALID(section) \\\n");
    fprintf(fp, "    ((section)->name != nullptr)\n\n");
}

/* ------------------------------------------------------------------------- */
void
adg_gen_cpp_write_action_table(struct adg_action** action_table, struct adg_section** section_table, FILE* fp)
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
            runafter_len = len + sizeof("_runafter_") > runafter_len ? len + sizeof("_runafter_") : runafter_len;
        else
            runafter_len = sizeof("nullptr") > runafter_len ? sizeof("nullptr") : runafter_len;

        if ((*actionp)->requires[0] != -1)
            requires_len = len + sizeof("_requires_") > requires_len ? len + sizeof("_requires_") : requires_len;
        else
            requires_len = sizeof("nullptr") > requires_len ? sizeof("nullptr") : requires_len;

        if ((*actionp)->metadeps[0] != -1)
            metadeps_len = len + sizeof("_metadeps_") > metadeps_len ? len + sizeof("_metadeps_") : metadeps_len;
        else
            metadeps_len = sizeof("nullptr") > metadeps_len ? sizeof("nullptr") : metadeps_len;
    }

    write_runafter_tables(action_table, fp);
    write_requires_tables(action_table, fp);
    write_metadeps_tables(action_table, fp);

    fprintf(fp, "Action::HU::HU() {}\n");
    fprintf(fp, "Action::HU::HU(Handler handler)\n");
    fprintf(fp, "    : standard(handler) {}\n");
    fprintf(fp, "Action::HU::HU(MetaHandler handler)\n");
    fprintf(fp, "    : meta(handler) {}\n");

    fprintf(fp, "#define IMPLICIT 0x01\n");
    fprintf(fp, "#define META     0x02\n");

    fprintf(fp, "static const Action actions_[] = {\n");
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
        fprintf(fp, "Action::HU(");
        padding -= fprintf(fp, "%s", action->func_name);
        fprintf(fp, "), ");
        while (padding-- > 0)
            putc(' ', fp);

        padding = runafter_len;
        if ((*actionp)->runafter[0] != -1)
            padding -= fprintf(fp, "%s_runafter_", action->func_name);
        else
            padding -= fprintf(fp, "nullptr");
        fprintf(fp, ", ");
        while (padding-- > 0)
            putc(' ', fp);

        padding = requires_len;
        if ((*actionp)->requires[0] != -1)
            padding -= fprintf(fp, "%s_requires_", action->func_name);
        else
            padding -= fprintf(fp, "nullptr");
        fprintf(fp, ", ");
        while (padding-- > 0)
            putc(' ', fp);

        padding = metadeps_len;
        if ((*actionp)->metadeps[0] != -1)
            padding -= fprintf(fp, "%s_metadeps_", action->func_name);
        else
            padding -= fprintf(fp, "nullptr");
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
    fprintf(fp, "    ((action)->fullOption != nullptr)\n\n");
}

/* ------------------------------------------------------------------------- */
void
adg_gen_cpp_write_argparse_preamble(FILE* fp)
{
    fprintf(fp, "%s", agd_argparse_cpp_preamble);
}

/* ------------------------------------------------------------------------- */
void
adg_gen_cpp_write_argparse_postamble(FILE* fp)
{
    fprintf(fp, "%s", agd_argparse_cpp_postamble);
}

/* ------------------------------------------------------------------------- */
void
adg_gen_cpp_write_helpers_forward_decl(FILE* fp)
{
    fprintf(fp, "static int findActionId(const char* fullOption);\n");
}

/* ------------------------------------------------------------------------- */
void
adg_gen_cpp_write_helpers_impl(FILE* fp)
{
    fprintf(fp, "ActionHandler ActionHandler::fromShortOption(char shortOption, const std::vector<std::string>& args)\n");
    fprintf(fp, "{\n");
    fprintf(fp, "    if (shortOption == '\\0')\n");
    fprintf(fp, "        return ActionHandler();\n");
    fprintf(fp, "    for (const Action* action = actions_; ACTION_VALID(action); ++action)\n");
    fprintf(fp, "        if (action->shortOption == shortOption)\n");
    fprintf(fp, "            return ActionHandler{args, int(action - actions_)};\n");
    fprintf(fp, "    return ActionHandler();\n");
    fprintf(fp, "}\n");

    fprintf(fp, "ActionHandler ActionHandler::fromFullOption(const std::string& fullOption, const std::vector<std::string>& args)\n");
    fprintf(fp, "{\n");
    fprintf(fp, "    for (const Action* action = actions_; ACTION_VALID(action); ++action)\n");
    fprintf(fp, "        if (fullOption == action->fullOption)\n");
    fprintf(fp, "            return ActionHandler{args, int(action - actions_)};\n");
    fprintf(fp, "    return ActionHandler();\n");
    fprintf(fp, "}\n");

    fprintf(fp, "ActionHandler ActionHandler::fromId(int actionId, const std::vector<std::string>& args)\n");
    fprintf(fp, "{\n");
    fprintf(fp, "    return ActionHandler{args, actionId};\n");
    fprintf(fp, "}\n");

    fprintf(fp, "static int findActionId(const char* fullOption)\n");
    fprintf(fp, "{\n");
    fprintf(fp, "    for (const Action* action = actions_; ACTION_VALID(action); ++action)\n");
    fprintf(fp, "        if (fullOption == action->fullOption)\n");
    fprintf(fp, "            return action - actions_;\n");
    fprintf(fp, "    return -1;\n");
    fprintf(fp, "}\n");
}
