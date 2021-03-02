#include "argdefgen/action.h"
#include "argdefgen/gen.h"
#include <string.h>

/* ------------------------------------------------------------------------- */
void
adg_gen_write_action_struct_cpp(FILE* fp)
{
    fprintf(fp, "struct Action\n{\n");
    fprintf(fp, "    struct MetaResult\n    {\n");
    fprintf(fp, "        Handler handler;\n");
    fprintf(fp, "        ArgList argList;\n");
    fprintf(fp, "    }\n\n");

    fprintf(fp, "    typedef std::vector<std::string> ArgList;\n");
    fprintf(fp, "    typedef bool (*Handler)(const ArgList& args);\n");
    fprintf(fp, "    typedef MetaResult (*MetaHandler)(const ArgList& args);\n\n");

    fprintf(fp, "    const char* fullOption;\n");
    fprintf(fp, "    const char* argDoc;\n");
    fprintf(fp, "    union {\n");
    fprintf(fp, "        Handler handler;\n");
    fprintf(fp, "        MetaHandler metaHandler;\n");
    fprintf(fp, "    };\n");
    fprintf(fp, "    int* requires;\n");
    fprintf(fp, "    struct { int l; int h; } argRange;\n");
    fprintf(fp, "    int priority;\n");
    fprintf(fp, "    bool isImplicit;\n");
    fprintf(fp, "    bool isMeta;\n");
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
void
adg_gen_write_action_table_cpp(struct adg_action** action_table, FILE* fp)
{
    struct adg_action** actionp;
    int padding;
    int longopt_len = 0, argdoc_len = 0, func_len = 0, requires_len = 0;
    for (actionp = action_table; *actionp; ++actionp)
    {
        int len = strlen((*actionp)->long_option);
        longopt_len = len > longopt_len ? len : longopt_len;
        len = strlen((*actionp)->arg_doc);
        argdoc_len = len > argdoc_len ? len : argdoc_len;
        len = strlen((*actionp)->func_name);
        func_len = len > func_len ? len : func_len;
        requires_len = len + 10 > requires_len ? len + 10 : requires_len;
    }

    write_requires_tables(action_table, fp);

    fprintf(fp, "static const Action actions_[] = {\n");
    for (actionp = action_table; *actionp; ++actionp)
    {
        struct adg_action* action = *actionp;
        padding = longopt_len;
        fprintf(fp, "    { \"");
        padding -= fprintf(fp, "%s", action->long_option);
        fprintf(fp, "\", ");
        while (padding--)
            putc(' ', fp);

        padding = argdoc_len;
        fprintf(fp, "\"");
        padding -= fprintf(fp, "%s", action->arg_doc);
        fprintf(fp, "\", ");
        while (padding--)
            putc(' ', fp);

        padding = func_len;
        putc('&', fp);
        padding -= fprintf(fp, "%s", action->func_name);
        fprintf(fp, ", ");
        while (padding--)
            putc(' ', fp);

        padding = requires_len;
        putc('&', fp);
        padding -= fprintf(fp, "%s_requires_", action->func_name);
        fprintf(fp, ", ");
        while (padding--)
            putc(' ', fp);

        fprintf(fp, "{%2d, %3d}, ", action->arg_range.l, action->arg_range.h);
        fprintf(fp, "%2d, ", action->priority);

        if (action->short_option != '\0')
            fprintf(fp, "'%c',", action->short_option);
        else
            fprintf(fp, " 0, ");

        fprintf(fp, action->is_implicit ? "true,  " : "false, ");
        fprintf(fp, action->is_meta ? "true,  " : "false, ");
        fprintf(fp, "\"%s\"", action->help);

        if (*(actionp+1))
            fprintf(fp, " },\n");
        else
            fprintf(fp, " }\n");
    }
    fprintf(fp, "};\n\n");
}
