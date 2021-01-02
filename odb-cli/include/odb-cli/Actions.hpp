#pragma once

#include <vector>
#include <string>

typedef bool (*HandlerFunc)(const std::vector<std::string>& args);

struct Action
{
    //! E.g. "my-full-option". Must not contain spaces.
    const char* fullOption;

    /*! Optional single character option, E.g. 'x'.
     * Set to '\0' if there is no short option.
     */
    char shortOption;

    /*!
     * String explaining what the expected arguments are, e.g. "<arg1> [arg2...]"
     * Can be nullptr if no doc is required.
     */
    const char* argDoc;

    /*!
     * The number of arguments an action is able to handle.
     * l must be >= 0
     * h must be >= l, or -1 if there is no upper limit
     */
    struct { int l; int h; } argRange;

    //! Lower number means higher priority
    int priority;

    /*!
     * Setting this to true causes the action to be executed automatically after
     * all higher priority actions are done. The action also won't appear in
     * the help string, and it cannot be specified on the command line.
     */
    bool implicit;

    //! Pointer to the action's handler
    HandlerFunc handler;

    //! Explanation on what this action does
    const char* doc;
};

bool parseCommandLine(int argc, char** argv);
