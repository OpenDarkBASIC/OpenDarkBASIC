#include "argdefgen/action.h"
#include "argdefgen/gen.h"
#include <string.h>

/* ------------------------------------------------------------------------- */
void
adg_gen_cpp_write_typedefs(FILE* fp)
{
    fprintf(fp, "typedef std::vector<std::string> ArgList;\n");
    fprintf(fp, "typedef bool (*Handler)(const ArgList& args);\n\n");

    fprintf(fp, "struct MetaHandlerResult\n{\n");
    fprintf(fp, "    int actionId = -1;\n");
    fprintf(fp, "    ArgList argList;\n");
    fprintf(fp, "};\n\n");

    fprintf(fp, "typedef MetaHandlerResult (*MetaHandler)(const ArgList& args);\n");
}

/* ------------------------------------------------------------------------- */
void
adg_gen_cpp_write_action_struct_def(FILE* fp)
{
    fprintf(fp, "struct Action\n{\n");
    fprintf(fp, "    const char* fullOption;\n");
    fprintf(fp, "    const char* argDoc;\n");
    fprintf(fp, "    const union HU {\n");
    fprintf(fp, "        Handler standard;\n");
    fprintf(fp, "        MetaHandler meta;\n");
    fprintf(fp, "    } handler;\n");
    fprintf(fp, "    const int* requires;\n");
    fprintf(fp, "    const int* metadeps;\n");
    fprintf(fp, "    const struct { int l; int h; } argRange;\n");
    fprintf(fp, "    const int priority;\n");
    fprintf(fp, "    const char shortOption;\n");
    fprintf(fp, "    const unsigned char type;\n");
    fprintf(fp, "    const char* help;\n");
    fprintf(fp, "};\n");
}

/* ------------------------------------------------------------------------- */
static void
write_requires_tables(struct adg_action** action_table, FILE* fp)
{
    struct adg_action** actionp;
    for (actionp = action_table; *actionp; ++actionp)
    {
        int* requires;
        if ((*actionp)->requires[0] == -1)
            continue;

        fprintf(fp, "static const int %s_requires_[] = {", (*actionp)->func_name);
        for (requires = (*actionp)->requires; *requires != -1; ++requires)
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
    struct adg_action** actionp;
    for (actionp = action_table; *actionp; ++actionp)
    {
        int* metadeps;
        if ((*actionp)->metadeps[0] == -1)
            continue;

        fprintf(fp, "static const int %s_metadeps_[] = {", (*actionp)->func_name);
        for (metadeps = (*actionp)->metadeps; *metadeps != -1; ++metadeps)
        {
            fprintf(fp, "%d, ", *metadeps);
        }
        fprintf(fp, "-1};\n");
    }
}

/* ------------------------------------------------------------------------- */
void
adg_gen_cpp_write_action_table(struct adg_action** action_table, FILE* fp)
{
    struct adg_action** actionp;
    int padding;
    int longopt_len = 0, argdoc_len = 0, func_len = 0, requires_len = 0, metadeps_len = 0;
    for (actionp = action_table; *actionp; ++actionp)
    {
        int len = strlen((*actionp)->long_option);
        longopt_len = len > longopt_len ? len : longopt_len;
        len = strlen((*actionp)->arg_doc);
        argdoc_len = len > argdoc_len ? len : argdoc_len;
        len = strlen((*actionp)->func_name);
        func_len = len > func_len ? len : func_len;

        if ((*actionp)->requires[0] != -1)
            requires_len = len + sizeof("_requires_") > requires_len ? len + sizeof("_requires_") : requires_len;
        else
            requires_len = sizeof("nullptr") > requires_len ? sizeof("nullptr") : requires_len;

        if ((*actionp)->metadeps[0] != -1)
            metadeps_len = len + sizeof("_metadeps_") > metadeps_len ? len + sizeof("_metadeps_") : metadeps_len;
        else
            metadeps_len = sizeof("nullptr") > metadeps_len ? sizeof("nullptr") : metadeps_len;
    }

    write_requires_tables(action_table, fp);
    write_metadeps_tables(action_table, fp);

    fprintf(fp, "static Action::HU toUnion(Handler handler) {\n");
    fprintf(fp, "    Action::HU u;\n");
    fprintf(fp, "    u.standard = handler;\n");
    fprintf(fp, "    return u;\n");
    fprintf(fp, "}\n");
    fprintf(fp, "static Action::HU toUnion(MetaHandler handler) {\n");
    fprintf(fp, "    Action::HU u;\n");
    fprintf(fp, "    u.meta = handler;\n");
    fprintf(fp, "    return u;\n");
    fprintf(fp, "}\n");

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
        fprintf(fp, "toUnion(&");
        padding -= fprintf(fp, "%s", action->func_name);
        fprintf(fp, "), ");
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

        if (action->short_option != '\0')
            fprintf(fp, "'%c',", action->short_option);
        else
            fprintf(fp, " 0, ");

        fprintf(fp, "%d, ", (action->is_implicit ? 1 : 0 ) | (action->is_meta ? 2 : 0));
        fprintf(fp, "\"%s\"", action->help);

        if (*(actionp+1))
            fprintf(fp, " },\n");
        else
            fprintf(fp, " }\n");
    }
    fprintf(fp, "};\n\n");
}
