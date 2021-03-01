#pragma once

union adg_node;

struct adg_action
{
    struct adg_action** runafter;
    struct adg_action** depends;

    char* section_name;
    char* action_name;

    char* long_option;
    char* arg_doc;
    char* help;
    char* func_name;

    struct {
        int l, h;
    } arg_range;

    char short_option;
    char is_implicit;
    char is_meta;
};

struct adg_action*
adg_action_create(void);

void
adg_action_destroy(struct adg_action* action);

struct adg_action**
adg_action_table_from_nodes(union adg_node* root);

void
adg_action_table_destroy(struct adg_action** action_table);
