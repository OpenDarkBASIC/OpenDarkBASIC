#pragma once

enum adg_node_type
{
    ADG_HELP,
    ADG_FUNC,
    ADG_RUNAFTER,
    ADG_ARG,
    ADG_OPTIONAL_ARG,
    ADG_ARGNAME,
    ADG_EXPLICIT_ACTION,
    ADG_IMPLICIT_ACTION,
    ADG_EXPLICIT_META_ACTION,
    ADG_IMPLICIT_META_ACTION,
    ADG_ACTIONATTRS,
    ADG_SECTION
};

union adg_node
{
    struct info {
        enum adg_node_type type;
    } info;

    struct base {
        struct info info;
        union adg_node* left;
        union adg_node* right;
    } base;

    struct actionattrs {
        struct info info;
        struct actionattrs* next;
        union adg_node* attr;
    } actionattrs;

    struct action_base {
        struct info info;
        struct action_base* next;
        struct actionattrs* attrs;
    } action_base;

    struct explicit_action {
        struct info info;
        struct action_base* next;
        struct actionattrs* attrs;
    } explicit_action;

    struct implicit_action {
        struct info info;
        struct action_base* next;
        struct actionattrs* attrs;
    } implicit_action;

    struct explicit_meta_action {
        struct info info;
        struct action_base* next;
        struct actionattrs* attrs;
    } explicit_meta_action;

    struct implicit_meta_action {
        struct info info;
        struct action_base* next;
        struct actionattrs* attrs;
    } implicit_meta_action;

    struct section {
        struct info info;
        struct section* next;
        union adg_node* actions;
        char* name;
    } section;

    struct runafter {
        struct info info;
        struct runafter* next;
        union adg_node* _padding;
        char* str;
    } runafter;

    struct argname {
        struct info info;
        struct argname* next;
        union adg_node* _padding;
        char* str;
    } argname;

    struct arg {
        struct info info;
        struct arg* next;
        struct argname* argnames;
    } arg;

    struct optional_arg {
        struct info info;
        struct optional_arg* next;
        struct argname* argnames;
        int continued;
    } optional_arg;

    struct {
        struct base base;
        char* text;
    } help;

    struct {
        struct base base;
        char* name;
    } func;
};

union adg_node* adg_node_new_help(char* str);
union adg_node* adg_node_new_func(char* str);
union adg_node* adg_node_new_runafter(union adg_node* next, char* str);
union adg_node* adg_node_new_arg(union adg_node* next, union adg_node* argnames);
union adg_node* adg_node_new_optional_arg(union adg_node* next, union adg_node* argnames, int continued);
union adg_node* adg_node_new_argname(union adg_node* next, char* name);
union adg_node* adg_node_new_explicit_action(char* name, union adg_node* attrs);
union adg_node* adg_node_new_implicit_action(char* name, union adg_node* attrs);
union adg_node* adg_node_new_explicit_meta_action(char* name, union adg_node* attrs);
union adg_node* adg_node_new_implicit_meta_action(char* name, union adg_node* attrs);
union adg_node* adg_node_new_actionattr(union adg_node* attr);
union adg_node* adg_node_new_section(union adg_node* action, char* name);

void adg_node_append_section(union adg_node* section, union adg_node* next);
void adg_node_append_action(union adg_node* action, union adg_node* next);
void adg_node_append_actionattr(union adg_node* actionattrs, union adg_node* next);

void adg_node_destroy(union adg_node* node);
void adg_node_destroy_recursive(union adg_node* node);

int adg_node_export_dot(union adg_node* root, const char* filename);
