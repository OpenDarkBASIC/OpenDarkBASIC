/* ------------------------------------------------------------------------- */
static void list_init(struct handler_list* list)
{
    list->entries = NULL;
    list->size = 0;
}

/* ------------------------------------------------------------------------- */
static void queue_init(struct handler_queue* q)
{
    q->front = NULL;
}

/* ------------------------------------------------------------------------- */
static struct queue_entry* dequeue(struct handler_queue* q)
{
    struct queue_entry* entry;
    entry = q->front;
    if (entry)
        q->front = entry->prev;
    return entry;
}

/* ------------------------------------------------------------------------- */
static void enqueue(struct handler_queue* q, struct queue_entry* entry, int priority)
{
    struct queue_entry* infront = NULL;
    struct queue_entry* behind = q->front;

    entry->prev = NULL;
    entry->priority = priority;
    while (behind && behind->priority <= priority)
    {
        infront = behind;
        behind = behind->prev;
    }
    if (infront)
    {
        infront->prev = entry;
        entry->prev = behind;
    }
    else
    {
        q->front = entry;
        entry->prev = behind;
    }
}

/* ------------------------------------------------------------------------- */
static struct queue_entry* peek_front(struct handler_queue* q)
{
    return q->front;
}

/* ------------------------------------------------------------------------- */
static int parse_full_option_init_handler(struct handler* handler, int argc, char** argv)
{
    /* skip "--" */
    char* str = &argv[0][2];

    /* Handle --option=arg1,arg2,... syntax separately */
    char* assignment = strchr(str, '=');
    if (assignment)
    {
        *assignment = '\0';
        handler->action_id = %prefixaction_id(str);
        if (handler->action_id == -1)
        {
            ADG_FPRINTF(stderr, "Error: Unrecognized command line option `--%s`\n", str);
            goto find_id_failed;
        }

        handler->args = split(assignment+1, ',');
        if (handler->args == NULL)
            goto split_assignmend_failed;

        *assignment = '=';
        return 1;  /* 1 arg in total from argv was parsed */

        split_assignmend_failed :
        find_id_failed          : *assignment = '=';
        return 0;
    }
    else  /* Handle --option arg1 arg2 ... syntax */
    {
        int i;
        int id = %prefixaction_id(str);
        if (id == -1)
        {
            ADG_FPRINTF(stderr, "Error: Unrecognized command line option `--%s`\n", str);
            return 0;
        }

        for (i = 0; i != argc - 1; ++i)
        {
            if (i == g_actions[id].arg_range.h)
                break;
            if (argv[i+1][0] == '-')
                break;

        }
    }
}

/* ------------------------------------------------------------------------- */
static int parse_full_option(int argc, char** argv, struct handler_list* list)
{
    int args_parsed;
    struct queue_entry* entry = (struct queue_entry*)malloc(sizeof *entry);
    if (entry == NULL)
        goto alloc_entry_failed;

    args_parsed = parse_full_option_init_handler(&entry->handler, argc, argv);
    if (args_parsed == 0)
        goto init_handler_failed;



    return args_parsed;

    init_handler_failed  : free(handler);
    alloc_entry_failed : return 0;
}

/* ------------------------------------------------------------------------- */
static int parse_short_options(int argc, char** argv, struct handler_list* list)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
static int parse_option(int argc, char** argv, struct handler_list* list)
{
    if (argv[0][0] == '-')
    {
        if (argv[0][1] == '-')
            return parse_full_option(argc, argv, list);
        else if (argv[0][1] != '\0')
            return parse_short_options(argc, argv, list);
    }

    ADG_FPRINTF(stderr, "Error: Unrecognized command line option `%s`\n", argv[0]);
    return 0;
}

/* ------------------------------------------------------------------------- */
int parse_command_line(int argc, char** argv)
{
    int i;
    struct handler_list list;
    struct handler_queue queue;
    list_init(&list);
    queue_init(&queue);

    program_name = argv[0];

    for (i = 1; i < argc; )
    {
        int processed = parse_option(argc - i, &argv[i], &list);
        if (processed == 0)
            return -1;
        i += processed;
    }

    return 0;
}
