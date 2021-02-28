#pragma once

struct adg_action
{
    struct adg_action** runafter;
    struct adg_action** depends;

    char* full_option;
    char* arg_doc;
    char* doc;
    char* func_name;

    struct {
        int l, h;
    } arg_range;

    int runafter_count;
    int depends_count;

    char is_implicit;
    char short_option;
};

struct adg_action*
adg_action_create(void);

void
adg_action_destroy(struct adg_action* action);

void
adg_action_destroy_recurse(struct adg_action* action);
