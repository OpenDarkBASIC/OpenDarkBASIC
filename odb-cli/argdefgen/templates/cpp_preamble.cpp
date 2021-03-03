#include <cstring>
#include <queue>

struct ActionHandler
{
    ArgList args;
    int actionId = -1;
    int priority;
};

struct ActionCompare
{
    bool operator()(const ActionHandler& a, const ActionHandler& b) {
        return a.priority > b.priority;
    }
};

typedef std::priority_queue<ActionHandler, std::vector<ActionHandler>, ActionCompare> ActionQueue;

static bool printHelp(const ArgList& args);
