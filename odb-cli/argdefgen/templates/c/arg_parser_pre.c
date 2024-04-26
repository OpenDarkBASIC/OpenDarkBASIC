static const char* program_name;

struct handler
{
    char** args;
    int action_id;
};

struct queue_entry
{
    struct queue_entry* prev;
    struct handler handler;
    int priority;
};

struct handler_queue
{
    struct queue_entry* front;
};

struct handler_list
{
    struct queue_entry* entries;
    int size;
};
