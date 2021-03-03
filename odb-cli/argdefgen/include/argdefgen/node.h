#pragma once

struct ADGLTYPE;

enum adg_node_type
{
    ADG_HEADER_PREAMBLE,
    ADG_HEADER_POSTAMBLE,
    ADG_SOURCE_PREAMBLE,
    ADG_SOURCE_POSTAMBLE,
    ADG_ACTION_TABLE,

    ADG_HELP,
    ADG_FUNC,
    ADG_RUNAFTER,
    ADG_REQUIRES,
    ADG_METADEP,
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

struct adg_node_location
{
    int l1;
    int c1;
    int l2;
    int c2;
};

union adg_node
{
    struct info {
        enum adg_node_type type;
        struct adg_node_location loc;
    } info;

    struct base {
        struct info info;
        union adg_node* left;
        union adg_node* right;
    } base;

    /* block nodes */

    struct block_base {
        struct info info;
        struct block_base* next;
        union adg_node* _padding;
    } block_base;

    struct {
        struct info info;
        union adg_node* next;
        union adg_node* _padding;
        char* text;
    } header_preamble;

    struct {
        struct info info;
        union adg_node* next;
        union adg_node* _padding;
        char* text;
    } header_postamble;

    struct {
        struct info info;
        union adg_node* next;
        union adg_node* _padding;
        char* text;
    } source_preamble;

    struct {
        struct info info;
        union adg_node* next;
        union adg_node* _padding;
        char* text;
    } source_postamble;

    struct {
        struct info info;
        union adg_node* next;
        union adg_node* sections;
    } action_table;

    /* action table nodes */

    struct actionattrs {
        struct info info;
        struct actionattrs* next;
        union adg_node* attr;
    } actionattrs;

    struct metadep {
        struct info info;
        struct metadep* next;
        union adg_node* _padding;
        char* str;
    } metadep;

    struct action_base {
        struct info info;
        struct action_base* next;
        struct actionattrs* attrs;
    } action_base;

    struct explicit_action {
        struct info info;
        struct action_base* next;
        struct actionattrs* attrs;
        char* longopt;
        char shortopt;
    } explicit_action;

    struct implicit_action {
        struct info info;
        struct action_base* next;
        struct actionattrs* attrs;
        char* name;
    } implicit_action;

    struct explicit_meta_action {
        struct info info;
        struct action_base* next;
        struct actionattrs* attrs;
        char* longopt;
        char shortopt;
    } explicit_meta_action;

    struct implicit_meta_action {
        struct info info;
        struct action_base* next;
        struct actionattrs* attrs;
        char* name;
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

    struct requires {
        struct info info;
        struct requires* next;
        union adg_node* _padding;
        char* str;
    } requires;

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

/* block nodes */
union adg_node* adg_node_new_header_preamble(char* text, struct ADGLTYPE* loc);
union adg_node* adg_node_new_header_postamble(char* text, struct ADGLTYPE* loc);
union adg_node* adg_node_new_source_preamble(char* text, struct ADGLTYPE* loc);
union adg_node* adg_node_new_source_postamble(char* text, struct ADGLTYPE* loc);
union adg_node* adg_node_new_action_table(union adg_node* sections, struct ADGLTYPE* loc);
void adg_node_append_block(union adg_node* block, union adg_node* next);
int adg_node_is_block(union adg_node* node);

/* action table nodes */
union adg_node* adg_node_new_help(char* str, struct ADGLTYPE* loc);
union adg_node* adg_node_new_func(char* str, struct ADGLTYPE* loc);
union adg_node* adg_node_new_runafter(union adg_node* next, char* str, struct ADGLTYPE* loc);
union adg_node* adg_node_new_requires(union adg_node* next, char* str, struct ADGLTYPE* loc);
union adg_node* adg_node_new_metadep(union adg_node* next, char* str, struct ADGLTYPE* loc);
union adg_node* adg_node_new_arg(union adg_node* next, union adg_node* argnames, struct ADGLTYPE* loc);
union adg_node* adg_node_new_optional_arg(union adg_node* next, union adg_node* argnames, int continued, struct ADGLTYPE* loc);
union adg_node* adg_node_new_argname(union adg_node* next, char* name, struct ADGLTYPE* loc);
union adg_node* adg_node_new_explicit_action(char* name, union adg_node* attrs, struct ADGLTYPE* loc);
union adg_node* adg_node_new_implicit_action(char* name, union adg_node* attrs, struct ADGLTYPE* loc);
union adg_node* adg_node_new_explicit_meta_action(char* name, union adg_node* attrs, struct ADGLTYPE* loc);
union adg_node* adg_node_new_implicit_meta_action(char* name, union adg_node* attrs, struct ADGLTYPE* loc);
union adg_node* adg_node_new_actionattr(union adg_node* attr, struct ADGLTYPE* loc);
union adg_node* adg_node_new_section(union adg_node* action, char* name, struct ADGLTYPE* loc);
int adg_node_is_action(union adg_node* node);

void adg_node_append_section(union adg_node* section, union adg_node* next);
void adg_node_append_action(union adg_node* action, union adg_node* next);
void adg_node_append_actionattr(union adg_node* actionattrs, union adg_node* next);

void adg_node_destroy(union adg_node* node);
void adg_node_destroy_recursive(union adg_node* node);

int adg_node_export_dot(union adg_node* root, const char* filename);

union adg_node* adg_node_find(union adg_node* node, enum adg_node_type type);
union adg_node* adg_node_find_action_matching(union adg_node* node, const char* action_node_name);

int adg_node_generate_help_action_if_not_available(union adg_node* root);
