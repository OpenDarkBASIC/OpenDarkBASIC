#include "argdefgen/node.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define MALLOC_AND_INIT(type)          \
    malloc(sizeof(union adg_node));    \
    if (node == NULL)                  \
        return NULL;                   \
    init_base(node, type)

/* ------------------------------------------------------------------------- */
static void
init_base(union adg_node* node, enum adg_node_type type)
{
    node->base.info.type = type;
    node->base.left = NULL;
    node->base.right = NULL;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_help(char* str)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_HELP);
    node->help.text = str;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_func(char* str)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_FUNC);
    node->func.name = str;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_runafter(union adg_node* next, char* str)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_RUNAFTER);
    if (next)
    {
        assert(next->info.type == ADG_RUNAFTER);
        node->runafter.next = &next->runafter;
    }

    node->runafter.str = str;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_arg(union adg_node* next, union adg_node* argnames)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_ARG);
    if (next)
    {
        assert(next->info.type == ADG_ARG || next->info.type == ADG_OPTIONAL_ARG);
        node->arg.next = &next->arg;
    }

    assert(argnames->info.type == ADG_ARGNAME);
    node->arg.argnames = &argnames->argname;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_optional_arg(union adg_node* next, union adg_node* argnames, int continued)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_OPTIONAL_ARG);
    if (next)
    {
        assert(next->info.type == ADG_OPTIONAL_ARG);
        node->optional_arg.next = &next->optional_arg;
    }

    assert(argnames->info.type == ADG_ARGNAME);
    node->optional_arg.argnames = &argnames->argname;
    node->optional_arg.continued = continued;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_argname(union adg_node* next, char* name)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_ARGNAME);
    if (next)
    {
        assert(next->info.type == ADG_ARGNAME);
        node->argname.next = &next->argname;
    }

    node->argname.str = name;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_explicit_action(char* name, union adg_node* attrs)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_EXPLICIT_ACTION);
    assert(attrs->info.type == ADG_ACTIONATTRS);
    node->explicit_action.attrs = &attrs->actionattrs;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_implicit_action(char* name, union adg_node* attrs)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_IMPLICIT_ACTION);
    assert(attrs->info.type == ADG_ACTIONATTRS);
    node->explicit_action.attrs = &attrs->actionattrs;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_explicit_meta_action(char* name, union adg_node* attrs)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_EXPLICIT_META_ACTION);
    assert(attrs->info.type == ADG_ACTIONATTRS);
    node->explicit_action.attrs = &attrs->actionattrs;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_implicit_meta_action(char* name, union adg_node* attrs)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_IMPLICIT_META_ACTION);
    assert(attrs->info.type == ADG_ACTIONATTRS);
    node->explicit_action.attrs = &attrs->actionattrs;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_actionattr(union adg_node* attr)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_ACTIONATTRS);
    node->actionattrs.attr = attr;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_section(union adg_node* action, char* name)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_SECTION);
    node->section.actions = action;
    node->section.name = name;
    return node;
}

/* ------------------------------------------------------------------------- */
void adg_node_append_section(union adg_node* section, union adg_node* next)
{
    union adg_node* last = section;

    assert(section->info.type == ADG_SECTION);
    assert(next->info.type == ADG_SECTION);
    assert(next->section.next == NULL);

    while (last->section.next != NULL)
        last = (union adg_node*)last->section.next;

    last->section.next = &next->section;
}

/* ------------------------------------------------------------------------- */
void adg_node_append_action(union adg_node* action, union adg_node* next)
{
    union adg_node* last = action;

    assert(action->info.type == ADG_EXPLICIT_ACTION
        || action->info.type == ADG_IMPLICIT_ACTION
        || action->info.type == ADG_EXPLICIT_META_ACTION
        || action->info.type == ADG_IMPLICIT_META_ACTION);
    assert(next->info.type == ADG_EXPLICIT_ACTION
        || next->info.type == ADG_IMPLICIT_ACTION
        || next->info.type == ADG_EXPLICIT_META_ACTION
        || next->info.type == ADG_IMPLICIT_META_ACTION);
    assert(next->action_base.next == NULL);

    while (last->action_base.next != NULL)
        last = (union adg_node*)last->action_base.next;

    last->action_base.next = &next->action_base;
}

/* ------------------------------------------------------------------------- */
void adg_node_append_actionattr(union adg_node* actionattrs, union adg_node* next)
{
    union adg_node* last = actionattrs;

    assert(actionattrs->info.type == ADG_ACTIONATTRS);
    assert(next->info.type == ADG_ACTIONATTRS);
    assert(next->actionattrs.next == NULL);

    while (last->actionattrs.next != NULL)
        last = (union adg_node*)last->actionattrs.next;

    last->actionattrs.next = &next->actionattrs;
}

/* ------------------------------------------------------------------------- */
void adg_node_destroy(union adg_node* node)
{
    switch (node->info.type)
    {
        case ADG_HELP                 : free(node->help.text); break;
        case ADG_FUNC                 : free(node->func.name); break;
        case ADG_RUNAFTER             : free(node->runafter.str); break;
        case ADG_ARG                  : break;
        case ADG_OPTIONAL_ARG         : break;
        case ADG_ARGNAME              : free(node->argname.str); break;
        case ADG_EXPLICIT_ACTION      : break;
        case ADG_IMPLICIT_ACTION      : break;
        case ADG_EXPLICIT_META_ACTION : break;
        case ADG_IMPLICIT_META_ACTION : break;
        case ADG_ACTIONATTRS          : break;
        case ADG_SECTION              : break;
    }

    free(node);
}

/* ------------------------------------------------------------------------- */
void adg_node_destroy_recursive(union adg_node* node)
{

}

/* ------------------------------------------------------------------------- */
static void write_connections(union adg_node* node, FILE* fp)
{
    switch (node->info.type)
    {
        case ADG_HELP                 : break;
        case ADG_FUNC                 : break;
        case ADG_RUNAFTER :
            if (node->runafter.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->runafter.next);
            break;
        case ADG_ARG :
            if (node->arg.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->arg.next);
            if (node->arg.argnames) fprintf(fp, "    N%p -> N%p [label=\"argnames\"];\n", node, node->arg.argnames);
            break;
        case ADG_OPTIONAL_ARG :
            if (node->optional_arg.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->optional_arg.next);
            if (node->optional_arg.argnames) fprintf(fp, "    N%p -> N%p [label=\"argnames\"];\n", node, node->optional_arg.argnames);
            break;
        case ADG_ARGNAME :
            if (node->argname.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->argname.next);
            break;
        case ADG_EXPLICIT_ACTION :
            if (node->explicit_action.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->explicit_action.next);
            if (node->explicit_action.attrs) fprintf(fp, "    N%p -> N%p [label=\"attrs\"];\n", node, node->explicit_action.attrs);
            break;
        case ADG_IMPLICIT_ACTION :
            if (node->implicit_action.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->implicit_action.next);
            if (node->implicit_action.attrs) fprintf(fp, "    N%p -> N%p [label=\"attrs\"];\n", node, node->implicit_action.attrs);
            break;
        case ADG_EXPLICIT_META_ACTION :
            if (node->explicit_meta_action.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->explicit_meta_action.next);
            if (node->explicit_meta_action.attrs) fprintf(fp, "    N%p -> N%p [label=\"attrs\"];\n", node, node->explicit_meta_action.attrs);
            break;
        case ADG_IMPLICIT_META_ACTION :
            if (node->implicit_meta_action.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->implicit_meta_action.next);
            if (node->implicit_meta_action.attrs) fprintf(fp, "    N%p -> N%p [label=\"attrs\"];\n", node, node->implicit_action.attrs);
            break;
        case ADG_ACTIONATTRS :
            if (node->actionattrs.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->actionattrs.next);
            if (node->actionattrs.attr) fprintf(fp, "    N%p -> N%p [label=\"attr\"];\n", node, node->actionattrs.attr);
            break;
        case ADG_SECTION :
            if (node->section.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->section.next);
            if (node->section.actions) fprintf(fp, "    N%p -> N%p [label=\"actions\"];\n", node, node->section.actions);
            break;
    }

    if (node->base.left)
        write_connections(node->base.left, fp);
    if (node->base.right)
        write_connections(node->base.right, fp);
};

/* ------------------------------------------------------------------------- */
static void write_names(union adg_node* node, FILE* fp)
{
    fprintf(fp, "    N%p [label=\"", node);
    switch (node->info.type)
    {
        case ADG_HELP                 : fprintf(fp, "help"); break;
        case ADG_FUNC                 : fprintf(fp, "func: %s", node->func.name); break;
        case ADG_RUNAFTER             : fprintf(fp, "runafter: %s", node->runafter.str); break;
        case ADG_ARG                  : fprintf(fp, "arg"); break;
        case ADG_OPTIONAL_ARG         : fprintf(fp, "optional arg: %s", node->optional_arg.continued ? "continued" : "not continued"); break;
        case ADG_ARGNAME              : fprintf(fp, "argname: %s", node->argname.str); break;
        case ADG_EXPLICIT_ACTION      : fprintf(fp, "explicit action"); break;
        case ADG_IMPLICIT_ACTION      : fprintf(fp, "implicit action"); break;
        case ADG_EXPLICIT_META_ACTION : fprintf(fp, "explicit meta action"); break;
        case ADG_IMPLICIT_META_ACTION : fprintf(fp, "implicit meta action"); break;
        case ADG_ACTIONATTRS          : fprintf(fp, "actionattrs"); break;
        case ADG_SECTION              : fprintf(fp, "section: %s", node->section.name); break;
    }
    fprintf(fp, "\"];\n");

    if (node->base.left)
        write_names(node->base.left, fp);
    if (node->base.right)
        write_names(node->base.right, fp);
}

/* ------------------------------------------------------------------------- */
int adg_node_export_dot(union adg_node* root, const char* filename)
{
    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Failed to open file %s\n", filename);
        return -1;
    }

    fprintf(fp, "digraph name {\n");
    write_connections(root, fp);
    write_names(root, fp);
    fprintf(fp, "}\n");

    fclose(fp);
    return 0;
}
