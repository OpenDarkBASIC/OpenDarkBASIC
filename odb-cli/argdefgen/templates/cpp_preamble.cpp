#include <cstring>
#include <queue>

struct ActionHandlerCompare
{
    bool operator()(const ActionHandler& a, const ActionHandler& b);
};

typedef std::priority_queue<ActionHandler, std::vector<ActionHandler>, ActionHandlerCompare> ActionQueue;
typedef std::vector<ActionHandler> ActionList;

static bool printHelp(const ArgList& args);
