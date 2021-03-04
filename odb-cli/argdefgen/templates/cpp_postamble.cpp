#if !defined(ADG_FPRINTF)
#   define ADG_FPRINTF(...) fprintf(__VA_ARGS__)
#endif
#if !defined(ADG_FPRINTF_COLOR)
#   define ADG_FPRINTF_COLOR(print_func, fp, color, fmt, ...) \
        print_func(fp, color fmt, __VA_ARGS__)
#endif
#if !defined(ADG_COLOR_RESET)
#   define ADG_COLOR_RESET    "\u001b[0m"
#endif
#if !defined(ADG_COLOR_HEADING1)
#define ADG_COLOR_HEADING1 "\u001b[1;37m"
#endif
#if !defined(ADG_COLOR_HEADING2)
#define ADG_COLOR_HEADING2 "\u001b[1;34m"
#endif
#if !defined(ADG_COLOR_LONGOPT)
#define ADG_COLOR_LONGOPT  "\u001b[1;32m"
#endif
#if !defined(ADG_COLOR_SHORTOPT)
#define ADG_COLOR_SHORTOPT "\u001b[1;36m"
#endif
#if !defined(ADG_COLOR_ARG)
#define ADG_COLOR_ARG      "\u001b[1;36m"
#endif
#if !defined(ADG_COLOR_ERROR)
#   define ADG_COLOR_ERROR "\u001b[1;31m"
#endif

#if !defined(ADG_HELP_EXAMPLES)
#   define ADG_HELP_EXAMPLES
#endif

static ActionQueue actionQueue_;
static std::string programName_;

// ----------------------------------------------------------------------------
static void split(std::vector<std::string>* strlist,
                  const std::string& str,
                  char delim=' ')
{
    std::size_t current, previous = 0;
    current = str.find(delim);
    while (current != std::string::npos)
    {
        strlist->push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find(delim, previous);
    }
    strlist->push_back(str.substr(previous, current - previous));
}

// ----------------------------------------------------------------------------
static void justifyWrap(std::vector<std::string>* lines,
                        const std::string& str,
                        int width,
                        char delim=' ')
{
    std::vector<std::string> wordList;
    split(&wordList, str);
    std::vector<std::string>::const_iterator lineStart = wordList.begin();
    std::vector<std::string>::const_iterator lineEnd = wordList.begin();

    auto appendAndJustify = [&lines, &lineStart, &lineEnd]() {
        lines->push_back("");
        for (std::vector<std::string>::const_iterator it = lineStart; it != lineEnd; ++it)
        {
            if (it != lineStart)
                lines->back().append(" ");
            lines->back().append(*it);
        }
    };

    int len = 0;
    while (lineEnd != wordList.end())
    {
        // No space left for another word
        if (len + (int)lineEnd->length() > width)
        {
            appendAndJustify();
            lineStart = lineEnd;
            len = 0;
        }
        else
        {
            len += lineEnd->length() + 1;  // +1 = space
            lineEnd++;
        }
    }

    if (lineStart != lineEnd)
        appendAndJustify();
}

// ----------------------------------------------------------------------------
static void printHelpAction(const Action* action)
{
    const int WRAP = 80;
    const int DOC_INDENT = 8;

    // Implicit actions are invisible to the user
    if (action->type & IMPLICIT)
        return;

    // Print short option, if it exists
    if (action->shortOption)
    {
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_SHORTOPT, "  -%c", action->shortOption);
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", ", ");
    }
    else
    {
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", "      ");
    }

    // Print full option
    ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_LONGOPT, "--%s ", action->fullOption);

    // Format argument documentation, if any
    for (const char* p = action->argDoc; *p; ++p)
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

    ArgList lines;
    justifyWrap(&lines, std::string(action->help), WRAP - DOC_INDENT);
    for (auto line = lines.begin(); line != lines.end(); ++line)
    {
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", "\n");
        for (int j = 0; j != DOC_INDENT; ++j)
        {
            ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", " ");
        }
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", line->c_str());
    }
    ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", "\n");
}
static void printAvailableSections()
{
    ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING1, "%s\n", "Available sections:");

    int padding = 0;
    for (const Section* section = sections_; SECTION_VALID(section); ++section)
    {
        std::size_t len = strlen(section->name);
        padding = padding < len ? len : padding;
    }
    for (const Section* section = sections_; SECTION_VALID(section); ++section)
    {
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING2, "  %-*s", padding, section->name);
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, " : %s\n", section->info);
    }
}
static void printSection(const char* sectionName)
{
    bool found = false;
    for (const Action* action = actions_; ACTION_VALID(action); ++action)
    {
        if (strcmp(sections_[action->sectionId].name, sectionName))
            continue;
        if (found == false)
        {
            ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING1, "%s", "Commands in section ");
            ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_ARG, "%s", sectionName);
            ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING1, "%s", ":\n");
        }
        printHelpAction(action);
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", "\n");
        found = true;
    }

    if (found == false)
    {
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_ERROR, "%s", "Error: ");
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "Unknown section `%s'\n\n", sectionName);
        printAvailableSections();
    }
}
static void printHelpAll()
{
    for (const Action* action = actions_; ACTION_VALID(action); ++action)
    {
        printHelpAction(action);
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", "\n");
    }
}

// ----------------------------------------------------------------------------
static bool printHelp(const ArgList& args)
{
    if (args.size() == 0)
    {
        // ------------------------------------------------------------------------
        // Usage
        // ------------------------------------------------------------------------

        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING1, "%s", "Usage:\n");
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "  %s ", programName_.c_str());
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", "[");
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_ARG, "%s", "options");
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", "]\n\n");

        // ------------------------------------------------------------------------
        // Examples
        // ------------------------------------------------------------------------

        ADG_HELP_EXAMPLES
    }

    // If there is only one section then we print everything. But if there are
    // multiple sections, only print the help doc if no args were specified.
    if (!SECTION_VALID(sections_ + 1) || (args.size() > 0 && args[0] == "all"))
    {
        ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING1, "%s\n", "Available options:");
        printHelpAll();
    }
    else if (SECTION_VALID(sections_ + 1) && args.size() == 0)
    {
        int actionId = findActionId("help");
        if (actionId == -1)
        {
            ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING1, "%s\n", "Available options:");
            printHelpAll();
        }
        else
        {
            printAvailableSections();
            ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_HEADING1, "\n%s\n", "Use --help to read more about a section:");
            printHelpAction(actions_ + actionId);
        }
    }
    else
    {
        printSection(args[0].c_str());
    }

    return false;
}

// ----------------------------------------------------------------------------
static int parseFullOption(int argc, char** argv, ActionQueue* queue)
{
    char* str = &argv[0][2];  // skip "--"
    auto processTable = [str, argc, argv](ActionQueue* queue) -> int
    {
        for (const Action* action = actions_; ACTION_VALID(action); ++action)
            if (strcmp(str, action->fullOption) == 0)
            {
                if (argc <= action->argRange.l)
                {
                    ADG_FPRINTF(stderr, "Error: Option %s expects at least %d argument%s\n", argv[0], action->argRange.l, action->argRange.l == 1 ? "" : "s");
                    return -1;
                }

                ActionHandler handler;
                handler.actionId = action - actions_;
                handler.priority = action->priority;
                for (int arg = 0; arg != action->argRange.h && arg != argc - 1 && argv[arg + 1][0] != '-'; ++arg)
                    handler.args.push_back(argv[arg + 1]);
                queue->push(handler);
                return handler.args.size() + 1;
            }
        return 0;
    };

    int argsProcessed;
    while (true)
    {
        if ((argsProcessed = processTable(queue)) > 0)
            return argsProcessed;
        else if (argsProcessed == -1)
            break;

        if (argsProcessed == 0)
            ADG_FPRINTF(stderr, "Error: Unrecognized command line option `%s`\n", argv[0]);

        break;
    }

    return 0;
}

// ----------------------------------------------------------------------------
static int parseShortOptions(int argc, char** argv, ActionQueue* queue)
{
    auto processTable = [argc, argv](ActionQueue* queue, const char* str) -> int
    {
        for (const Action* action = actions_; ACTION_VALID(action); ++action)
            if (action->shortOption == str[0])
            {
                if (action->argRange.l > 0 && str[1] != '\0')
                {
                    ADG_FPRINTF(stderr, "Option `-%c` must be at end of short option list (before `-%c`)\n", *str, str[1]);
                    return -1;
                }
                if (argc <= action->argRange.l)
                {
                    ADG_FPRINTF(stderr, "Error: Option %s expects at least %d argument%s\n", argv[0], action->argRange.l, action->argRange.l == 1 ? "" : "s");
                    return -1;
                }

                ActionHandler handler;
                handler.actionId = action - actions_;
                handler.priority = action->priority;
                if (str[1] == '\0')
                {
                    for (int arg = 0; arg != action->argRange.h && arg != argc - 1 && argv[arg + 1][0] != '-'; ++arg)
                        handler.args.push_back(argv[arg + 1]);
                }
                queue->push(handler);
                return handler.args.size() + 1;
            }

        return 0;
    };

    for (char* str = &argv[0][1]; *str; ++str)
    {
        int argsProcessed;
        if ((argsProcessed = processTable(queue, str)) > 1)
            return argsProcessed;
        else if (argsProcessed == 1)
            continue;

        if (argsProcessed == 0)
            ADG_FPRINTF(stderr, "Error: Unrecognized command line option `-%c`\n", *str);
        return 0;
    }

    return 1;
}

// ----------------------------------------------------------------------------
static int parseOption(int argc, char** argv, ActionQueue* queue)
{
    if (argv[0][0] == '-')
    {
        if (argv[0][1] == '-')
            return parseFullOption(argc, argv, queue);
        else
            return parseShortOptions(argc, argv, queue);
    }

    ADG_FPRINTF(stderr, "Error: Unrecognized command line option `%s`\n", argv[0]);
    return 0;
}

// ----------------------------------------------------------------------------
static void addImplicitActions(ActionQueue* queue)
{
    if (queue->empty())
        return;

    // First, remove all existing implicit actions in the queue
    ActionQueue newExplicitQueue;
    int lastPriority = 0;
    while (!queue->empty())
    {
        const ActionHandler& handler = queue->top();
        if (lastPriority < handler.priority)
            lastPriority = handler.priority;
        if (!(actions_[handler.actionId].type & IMPLICIT))
            newExplicitQueue.push(handler);
        queue->pop();
    }
    *queue = std::move(newExplicitQueue);

    for (const Action* action = actions_; ACTION_VALID(action); ++action)
    {
        if (!(action->type & IMPLICIT))
            continue;

        // Only add an implicit action if there is an action in the queue that
        // depends on it. These would be actions that strictly have a lower
        // value
        if (action->priority >= lastPriority)
            continue;

        ActionHandler handler;
        handler.actionId = action - actions_;
        handler.priority = action->priority;
        queue->push(handler);
    }
}

// ----------------------------------------------------------------------------
bool parseCommandLine(int argc, char** argv)
{
    ActionQueue queue;
    programName_ = argv[0];

    for (int i = 1; i < argc; )
    {
        int processed = parseOption(argc - i, &argv[i], &queue);
        if (processed == 0)
            return false;
        i += processed;
    }

    // Find the priority of the help action
    int helpPriority = 0;
    int helpActionId = -1;
    for (const Action* action = actions_; ACTION_VALID(action); ++action)
        if (strcmp(action->fullOption, "help") == 0)
        {
            helpActionId = action - actions_;
            helpPriority = action->priority;
            break;
        }

    // Find the priority of the action that will be executed last
    ActionQueue copy(queue);
    int lastPriority = 0;
    while (!copy.empty())
    {
        if (lastPriority < copy.top().priority)
            lastPriority = copy.top().priority;
        copy.pop();
    }

    // Be nice and show help if no meaningful actions are going to be run.
    // We assume that all actions that are run before the help action are not
    // meaningful
    if (queue.empty() || lastPriority < helpPriority)
    {
        // or not
        if (helpActionId == -1)
            return true;

        ActionHandler handler;
        handler.actionId = helpActionId;
        handler.priority = helpPriority;
        queue.push(handler);
    }

    addImplicitActions(&queue);

    // Process all actions
    while (!queue.empty())
    {
        const ActionHandler& handler = queue.top();
        const Action& action = actions_[handler.actionId];
        if (action.type & META)
        {
            MetaHandlerResult result = action.handler.meta(handler.args);
            if (result.actionId == -1)
                return false;

            const int* metadepId = action.metadeps;
            while (1)
            {
                if (result.actionId == *metadepId++)
                    break;
                if (*metadepId == -1)
                {
                    if (result.actionId < static_cast<int>(sizeof(actions_) / sizeof(*actions_)))
                        ADG_FPRINTF(stderr, "Error: Meta-action `%s' tried to execute `%s' without specifying it in its meta dependency list\n", action.fullOption, actions_[result.actionId].fullOption);
                    else
                        ADG_FPRINTF(stderr, "Error: Meta-action `%s' returned an actionId %d which is out of range\n", action.fullOption, result.actionId);
                    return false;
                }
            }

            ActionHandler handler;
            handler.actionId = result.actionId;
            handler.args = result.args;
            handler.priority = actions_[result.actionId].priority;
            queue.push(handler);
        }
        else
        {
            if (action.handler.standard(handler.args) == false)
                return false;
        }
        queue.pop();
    }

    return true;
}

