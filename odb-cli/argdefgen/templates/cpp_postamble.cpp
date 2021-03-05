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

static std::string programName_;

// ----------------------------------------------------------------------------
bool ActionHandlerCompare::operator()(const ActionHandler& a, const ActionHandler& b)
{
    return actions_[a.actionId].priority > actions_[b.actionId].priority;
}

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
static void printHelpForAction(const Action* action)
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
    ADG_FPRINTF_COLOR(ADG_FPRINTF, stdout, ADG_COLOR_RESET, "%s", "\n\n");
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
        printHelpForAction(action);
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
        printHelpForAction(action);
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
            printHelpForAction(actions_ + actionId);
        }
    }
    else
    {
        printSection(args[0].c_str());
    }

    return false;
}

// ----------------------------------------------------------------------------
static int parseFullOption(int argc, char** argv, ActionList* list)
{
    // skip "--"
    const char* str = &argv[0][2];

    // Handle --option=arg1,arg2,... syntax separately
    const char* assignment = strchr(str, '=');
    if (assignment)
    {
        std::string name(str, assignment - str);
        auto handler = ActionHandler::fromFullOption(name);
        if (handler.actionId == -1)
        {
            ADG_FPRINTF(stderr, "Error: Unrecognized command line option `--%s'\n", name.c_str());
            return 0;
        }

        assignment++;
        split(&handler.args, assignment, ',');
        list->push_back(handler);
    }
    else  // handle --option arg1 arg2 ... syntax
    {
        auto handler = ActionHandler::fromFullOption(str);
        if (handler.actionId == -1)
        {
            ADG_FPRINTF(stderr, "Error: Unrecognized command line option `%s'\n", argv[0]);
            return 0;
        }

        const Action& action = actions_[handler.actionId];
        for (int i = 0; i != argc - 1; ++i)
        {
            if (i == action.argRange.h)
                break;
            if (argv[i+1][0] == '-')
                break;
            handler.args.push_back(argv[i + 1]);
        }
        list->push_back(handler);
    }

    const ActionHandler& handler = list->back();
    const Action& action = actions_[handler.actionId];
    if (handler.args.size() < action.argRange.l)
    {
        ADG_FPRINTF(stderr, "Error: Option `--%s' expects at least %d argument%s\n",
                    action.fullOption, action.argRange.l, action.argRange.l == 1 ? "" : "s");
        return 0;
    }
    if (handler.args.size() > action.argRange.h)
    {
        ADG_FPRINTF(stderr, "Error: Option `--%s' accepts at most %d argument%s\n",
                    action.fullOption, action.argRange.h, action.argRange.h == 1 ? "" : "s");
        return 0;
    }

    if (assignment)  // We only processed one argument
        return 1;

    return handler.args.size() + 1;
}

// ----------------------------------------------------------------------------
static int parseShortOptions(int argc, char** argv, ActionList* list)
{
    // skip "-"
    // Note: there is guaranteed to be at least one character after "-"
    const char* str = &argv[0][1];
    while (str[0])
    {
        auto handler = ActionHandler::fromShortOption(str[0]);
        if (handler.actionId == -1)
        {
            ADG_FPRINTF(stderr, "Error: Unrecognized short option `-%c'\n", str[0]);
            return 0;
        }

        // Interpret the rest of the string as the first argument if the action
        // has a minimum arg count requirement
        const Action& action = actions_[handler.actionId];
        if (action.argRange.l > 0)
        {
            if (str[1] != '\0')
                handler.args.push_back(str + 1);
            for (int i = 0; i != argc - 1; ++i)
            {
                if (handler.args.size() == action.argRange.h)
                    break;
                if (argv[i+1][0] == '-')
                    break;
                handler.args.push_back(argv[i + 1]);
            }
            list->push_back(handler);

            if (str[1] != '\0')
                return handler.args.size();
            return handler.args.size() + 1;
        }

        list->push_back(handler);
        str++;
    }

    return 1;
}

// ----------------------------------------------------------------------------
static int parseOption(int argc, char** argv, ActionList* list)
{
    if (argv[0][0] == '-')
    {
        if (argv[0][1] == '-')
            return parseFullOption(argc, argv, list);
        else if (argv[0][1] != '\0')
            return parseShortOptions(argc, argv, list);
    }

    ADG_FPRINTF(stderr, "Error: Unrecognized command line option `%s`\n", argv[0]);
    return 0;
}

// ----------------------------------------------------------------------------
static bool actionIsInList(const ActionList& list, int actionId)
{
    for (const auto& handler : list)
        if (handler.actionId == actionId)
            return true;
    return false;
}
static void addImplicitRunafterDependencies(ActionList* list, int actionId)
{
    if ((actions_[actionId].type & IMPLICIT) && actionIsInList(*list, actionId) == false)
        list->push_back(ActionHandler::fromId(actionId));

    for (const int* child = actions_[actionId].runafter; child && *child != -1; ++child)
        addImplicitRunafterDependencies(list, *child);
    for (const int* child = actions_[actionId].metadeps; child && *child != -1; ++child)
        addImplicitRunafterDependencies(list, *child);
}
static void addImplicitDependencies(ActionList* list)
{
    for (std::size_t i = 0; i < list->size(); ++i)
    {
        const Action& action = actions_[(*list)[i].actionId];
        for (const int* child = action.runafter; child && *child != -1; ++child)
            addImplicitRunafterDependencies(list, *child);
        for (const int* child = action.metadeps; child && *child != -1; ++child)
            addImplicitRunafterDependencies(list, *child);
    }
}

// ----------------------------------------------------------------------------
static void tryToHelp(ActionList* list)
{
    // Find the priority of the help action
    int helpActionId = findActionId("help");
    int helpPriority = helpActionId != -1 ? actions_[helpActionId].priority : 0;

    // Find the priority of the lowest priority action (higher numbers means lower priority)
    int lastPriority = [](const ActionList& list) -> int {
        int priority = 0;
        for (const auto& handler : list)
        {
            if (priority < actions_[handler.actionId].priority)
                priority = actions_[handler.actionId].priority;
        }
        return priority;
    }(*list);

    // Be nice and show help if no meaningful actions are going to be run.
    // We assume that all actions that are run before the help action are not
    // meaningful. Small caveat: Meta actions can have a lower priority than
    // the help action, but can trigger higher priority actions to be run.
    // Therefore, if any meta actions are in the list, skip trying to add
    // help
    for (const auto& handler : *list)
        if (actions_[handler.actionId].type & META)
            return;

    if (list->size() == 0 || lastPriority < helpPriority)
        if (helpActionId != -1) // or maybe not if there is no help action
        {
            list->push_back(ActionHandler::fromId(helpActionId));
            addImplicitDependencies(list);
        }
}

// ----------------------------------------------------------------------------
bool parseCommandLine(int argc, char** argv)
{
    ActionList list;
    programName_ = argv[0];

    for (int i = 1; i < argc; )
    {
        int processed = parseOption(argc - i, &argv[i], &list);
        if (processed == 0)
            return false;
        i += processed;
    }

    // Ensure that all requires dependencies are satisfied
    for (const auto& action : list)
    {
        if (actions_[action.actionId].requires == nullptr)
            continue;

        // Because this action has a requires dependency list, each actionId in
        // that list must have been specified on the command line
        auto isInList = [&list](int actionId) ->bool {
            for (const auto& action : list)
                if (action.actionId == actionId)
                    return true;
            return false;
        };
        for (const int* child = actions_[action.actionId].requires; *child != -1; ++child)
            if (isInList(*child) == false)
            {
                ADG_FPRINTF(stderr, "Error: Option `--%s' requires --`%s'\n",
                            actions_[action.actionId].fullOption, actions_[*child].fullOption);
                return false;
            }
    }

    addImplicitDependencies(&list);
    tryToHelp(&list);

    // Move all handlers from the list into a priority queue and make sure
    // actions with equal priority are executed in the order they are in the
    // list
    ActionQueue queue;
    for (auto it = list.rbegin(); it != list.rend(); ++it)
        queue.push(std::move(*it));

    // Process all actions
    while (!queue.empty())
    {
        ActionHandler handler = queue.top();
        queue.pop();

        const Action& action = actions_[handler.actionId];
        if (action.type & META)
        {
            ActionHandler result = action.handler.meta(handler.args);
            if (result.actionId == -1)
                return false;

            // Ensure meta dependency list is satisfied
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

            // Check that the correct number of arguments were returned
            const Action& resultAction = actions_[result.actionId];
            if (result.args.size() < resultAction.argRange.l)
            {
                ADG_FPRINTF(stderr, "Error: Meta-action `--%s' returned %d arguments, but `--%s' expects at least %d argument%s\n",
                            action.fullOption, (int)result.args.size(),
                            resultAction.fullOption, resultAction.argRange.l, resultAction.argRange.l == 1 ? "" : "s");
                return 0;
            }
            if (result.args.size() > resultAction.argRange.h)
            {
                ADG_FPRINTF(stderr, "Error: Meta-action `--%s' returned %d arguments, but `--%s' accepts at most %d argument%s\n",
                            action.fullOption, (int)result.args.size(),
                            resultAction.fullOption, resultAction.argRange.h, resultAction.argRange.h == 1 ? "" : "s");
                return 0;
            }

            queue.push(result);
        }
        else
        {
            if (action.handler.standard(handler.args) == false)
                return false;
        }
    }

    return true;
}
