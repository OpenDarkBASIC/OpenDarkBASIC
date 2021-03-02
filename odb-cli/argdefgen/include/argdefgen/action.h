#pragma once

union adg_node;

struct adg_action
{
    int* runafter;
    int* requires;
    int* metadeps;

    char* section_name;
    char* action_name;

    char* long_option;
    char* arg_doc;
    char* help;
    char* func_name;

    struct {
        int l, h;
    } arg_range;

    int priority;
    char short_option;
    char is_implicit;
    char is_meta;
};

struct adg_action*
adg_action_create(void);

void
adg_action_destroy(struct adg_action* action);

struct adg_action**
adg_action_table_new_empty(void);

struct adg_action**
adg_action_table_from_nodes(union adg_node* root);

void
adg_action_table_destroy(struct adg_action** action_table);

void
adg_action_table_export_dot(struct adg_action** action_table, const char* filename);
