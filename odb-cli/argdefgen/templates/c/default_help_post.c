#if !defined(ADG_HELP_EXAMPLES)
#   define ADG_HELP_EXAMPLES
#endif

/* ------------------------------------------------------------------------- */
static void print_help_for_action(const struct %prefixaction* action)
{
    char** lines;
    char** line;
    const int WRAP = 80;
    const int DOC_INDENT = 8;

    // Implicit actions are invisible to the user
    if (action->type & IMPLICIT)
        return;

    // Print short option, if it exists
    if (action->short_option)
    {
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_SHORTOPT, "  -%c", action->short_option);
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", ", ");
    }
    else
    {
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", "      ");
    }

    // Print full option
    ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_LONGOPT, "--%s ", action->full_option);

    // Format argument documentation, if any
    for (const char* p = action->arg_doc; *p; ++p)
    {
        if (*p == '<' || *p == '>' || *p == '[' || *p == ']' || *p == '|' || *p == '.')
        {
            ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%c", *p);
        }
        else
        {
            ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_ARG, "%c", *p);
        }
    }

    lines = justify_wrap(action->help, WRAP - DOC_INDENT);
    for (line = lines; *line; ++line)
    {
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", "\n");
        for (int j = 0; j != DOC_INDENT; ++j)
        {
            ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", " ");
        }
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", *line);
    }
    ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", "\n\n");
    free(lines);
}

/* ------------------------------------------------------------------------- */
static void print_available_sections()
{
    ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING1, "%s\n", "Available sections:");

    int padding = 0;
    for (const struct %prefixsection* section = g_sections; SECTION_VALID(section); ++section)
    {
        std::size_t len = strlen(section->name);
        padding = padding < len ? len : padding;
    }
    for (const struct %prefixsection* section = g_sections; SECTION_VALID(section); ++section)
    {
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING2, "  %-*s", padding, section->name);
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, " : %s\n", section->info);
    }
}

/* ------------------------------------------------------------------------- */
static void print_section(const char* section_name)
{
    bool found = false;
    for (const struct %prefixaction* action = g_actions; ACTION_VALID(action); ++action)
    {
        if (strcmp(g_sections[action->section_id].name, section_name))
            continue;
        if (found == false)
        {
            ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING1, "%s", "Commands in section ");
            ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_ARG, "%s", section_name);
            ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING1, "%s", ":\n");
        }
        print_help_for_action(action);
        found = true;
    }

    if (found == false)
    {
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_ERROR, "%s", "Error: ");
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "Unknown section `%s'\n\n", section_name);
        print_available_sections();
    }
}

/* ------------------------------------------------------------------------- */
static void print_all_help()
{
    for (const struct %prefixaction* action = g_actions; ACTION_VALID(action); ++action)
    {
        print_help_for_action(action);
    }
}

/* ------------------------------------------------------------------------- */
static int print_help(char** args)
{
    if (args[0] == NULL)
    {
        /* ----------------------------------------------------------------- */
        // Usage
        /* ----------------------------------------------------------------- */

        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING1, "%s", "Usage:\n");
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "  %s ", program_name);
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", "[");
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_ARG, "%s", "options");
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", "]\n\n");

        /* ----------------------------------------------------------------- */
        // Examples
        /* ----------------------------------------------------------------- */

        ADG_HELP_EXAMPLES
    }

    // If there is only one section then we print everything. But if there are
    // multiple sections, only print the help doc if no args were specified.
    if (!SECTION_VALID(g_sections + 1) || (args[0] != NULL && strcmp(args[0], "all") == 0))
    {
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING1, "%s\n", "Available options:");
        print_all_help();
    }
    else if (SECTION_VALID(g_sections + 1) && args[0] == NULL)
    {
        int aid = %prefixaction_id("help");
        if (aid == -1)
        {
            ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING1, "%s\n", "Available options:");
            print_all_help();
        }
        else
        {
            print_available_sections();
            ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING1, "\n%s\n", "Use --help to read more about a section:");
            print_help_for_action(g_actions + aid);
        }
    }
    else
    {
        print_section(args[0]);
    }

    return false;
}
