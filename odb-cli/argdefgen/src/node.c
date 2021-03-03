#include "argdefgen/node.h"
#include "argdefgen/parser.y.h"
#include "argdefgen/str.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>

#define MALLOC_AND_INIT(type, loc)     \
    malloc(sizeof(union adg_node));    \
    if (node == NULL)                  \
        return NULL;                   \
    init_base(node, type, loc)

/* ------------------------------------------------------------------------- */
static void
init_base(union adg_node* node, enum adg_node_type type, struct ADGLTYPE* loc)
{
    node->base.info.type = type;
    node->base.info.loc.l1 = loc->first_line;
    node->base.info.loc.c1 = loc->first_column;
    node->base.info.loc.l2 = loc->last_line;
    node->base.info.loc.c2 = loc->last_column;
    node->base.left = NULL;
    node->base.right = NULL;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_header_preamble(char* text, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_HEADER_PREAMBLE, loc);
    node->header_preamble.text = text;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_header_postamble(char* text, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_HEADER_POSTAMBLE, loc);
    node->header_postamble.text = text;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_source_preamble(char* text, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_SOURCE_PREAMBLE, loc);
    node->source_preamble.text = text;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_source_postamble(char* text, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_SOURCE_POSTAMBLE, loc);
    node->source_postamble.text = text;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_action_table(union adg_node* sections, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_ACTION_TABLE, loc);
    node->action_table.sections = sections;
    return node;
}

/* ------------------------------------------------------------------------- */
void adg_node_append_block(union adg_node* block, union adg_node* next)
{
    union adg_node* last = block;

    assert(adg_node_is_block(block));
    assert(adg_node_is_block(next));
    assert(next->block_base.next == NULL);

    while (last->block_base.next != NULL)
        last = (union adg_node*)last->block_base.next;

    last->block_base.next = &next->block_base;
}

/* ------------------------------------------------------------------------- */
int adg_node_is_block(union adg_node* node)
{
    return node->info.type == ADG_HEADER_PREAMBLE
        || node->info.type == ADG_HEADER_POSTAMBLE
        || node->info.type == ADG_SOURCE_PREAMBLE
        || node->info.type == ADG_SOURCE_POSTAMBLE
        || node->info.type == ADG_ACTION_TABLE;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_help(char* str, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_HELP, loc);
    node->help.text = str;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_sectioninfo(char* str, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_SECTIONINFO, loc);
    node->sectioninfo.text = str;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_func(char* str, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_FUNC, loc);
    node->func.name = str;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_runafter(union adg_node* next, char* str, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_RUNAFTER, loc);
    if (next)
    {
        assert(next->info.type == ADG_RUNAFTER);
        node->runafter.next = &next->runafter;
    }

    node->runafter.str = str;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_requires(union adg_node* next, char* str, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_REQUIRES, loc);
    if (next)
    {
        assert(next->info.type == ADG_REQUIRES);
        node->runafter.next = &next->runafter;
    }

    node->runafter.str = str;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_metadep(union adg_node* next, char* str, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_METADEP, loc);
    if (next)
    {
        assert(next->info.type == ADG_METADEP);
        node->runafter.next = &next->runafter;
    }

    node->runafter.str = str;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_arg(union adg_node* next, union adg_node* argnames, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_ARG, loc);
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
union adg_node* adg_node_new_optional_arg(union adg_node* next, union adg_node* argnames, int continued, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_OPTIONAL_ARG, loc);
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
union adg_node* adg_node_new_argname(union adg_node* next, char* name, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_ARGNAME, loc);
    if (next)
    {
        assert(next->info.type == ADG_ARGNAME);
        node->argname.next = &next->argname;
    }

    node->argname.str = name;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_explicit_action(char* name, union adg_node* attrs, struct ADGLTYPE* loc)
{
    char* longopt_end;

    union adg_node* node = MALLOC_AND_INIT(ADG_EXPLICIT_ACTION, loc);
    assert(attrs->info.type == ADG_ACTIONATTR);
    node->explicit_action.attrs = &attrs->actionattr;

    longopt_end = strchr(name, '(');
    *longopt_end = '\0'; /* just terminate string here so longopt is correct */
    node->explicit_action.longopt = name;
    node->explicit_action.shortopt = longopt_end[1] == ')' ? '\0' : longopt_end[1];

    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_implicit_action(char* name, union adg_node* attrs, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_IMPLICIT_ACTION, loc);
    assert(attrs->info.type == ADG_ACTIONATTR);
    node->implicit_action.attrs = &attrs->actionattr;
    node->implicit_action.name = name;
    return node;
}

/* ------------------------------------------------------------------------- */
static char* parse_meta_arglist_expect_secname(char** str)
{
    char* start = *str;
    char* end = *str;
    while (isalnum(*end) || *end == '_' || *end == '-')
        ++end;
    (*str) = end;

    return adg_str_dup_range(start, 0, end - start);
}
static union adg_node* parse_meta_arglist_expect_stringlist(char** str, struct ADGLTYPE* loc)
{
    union adg_node* first;
    union adg_node* next;
    union adg_node* metadep;

    char* secname = parse_meta_arglist_expect_secname(str);
    first = next = adg_node_new_metadep(NULL, secname, loc);

    while (1)
    {
        while (!isalnum(**str))
        {
            if (**str == ']')
                return first;
            if (**str != ',' && **str != ' ')
            {
                fprintf(stderr, "Unexpected '%c', expected ',' or ' '\n", **str);
                adg_node_destroy_recursive(first);
                return NULL;
            }

            (*str)++;
        }

        secname = parse_meta_arglist_expect_secname(str);
        metadep = adg_node_new_metadep(NULL, secname, loc);
        next->metadep.next = &metadep->metadep;
        next = metadep;
    }

    return NULL;
}
static union adg_node* parse_meta_arglist(char* str, struct ADGLTYPE* loc)
{
    union adg_node* metadeps;
    if (*str++ != '[')
    {
        fprintf(stderr, "Error: Unexpected `%c', expected `['\n", str[-1]);
        goto missing_open_square_bracket;
    }

    metadeps = parse_meta_arglist_expect_stringlist(&str, loc);
    if (metadeps == NULL)
        goto parse_stringlist_failed;

    if (*str++ != ']')
    {
        fprintf(stderr, "Error: Unexpected `%c', expected `]'\n", str[-1]);
        goto missing_closed_square_bracket;
    }

    return metadeps;

    missing_closed_square_bracket : adg_node_destroy_recursive(metadeps);
    parse_stringlist_failed       :
    missing_open_square_bracket   : return NULL;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_explicit_meta_action(char* name, union adg_node* attrs, struct ADGLTYPE* loc)
{
    char* longopt_end;
    union adg_node* metadeps;

    union adg_node* node = MALLOC_AND_INIT(ADG_EXPLICIT_META_ACTION, loc);
    assert(attrs->info.type == ADG_ACTIONATTR);

    longopt_end = strchr(name, '(');
    *longopt_end = '\0'; /* just terminate string here so longopt is correct */

    metadeps = parse_meta_arglist(strchr(&longopt_end[1], '['), loc);
    if (metadeps == NULL)
    {
        adg_node_destroy(node);
        return NULL;
    }

    metadeps = adg_node_new_actionattr(metadeps, loc);
    metadeps->actionattr.next = &attrs->actionattr;

    node->explicit_meta_action.attrs = &metadeps->actionattr;
    node->explicit_meta_action.longopt = name;
    node->explicit_meta_action.shortopt = longopt_end[1] == ')' ? '\0' : longopt_end[1];

    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_implicit_meta_action(char* name, union adg_node* attrs, struct ADGLTYPE* loc)
{
    union adg_node* metadeps;
    union adg_node* node = MALLOC_AND_INIT(ADG_IMPLICIT_META_ACTION, loc);
    assert(attrs->info.type == ADG_ACTIONATTR);

    metadeps = parse_meta_arglist(strchr(name, '['), loc);
    if (metadeps == NULL)
    {
        adg_node_destroy(node);
        return NULL;
    }

    metadeps = adg_node_new_actionattr(metadeps, loc);
    metadeps->actionattr.next = &attrs->actionattr;

    node->implicit_meta_action.attrs = &metadeps->actionattr;
    node->implicit_meta_action.name = name;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_actionattr(union adg_node* attr, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_ACTIONATTR, loc);
    node->actionattr.attr = attr;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_section(union adg_node* attrs, char* name, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_SECTION, loc);
    node->section.attrs = attrs;
    node->section.name = name;
    return node;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_new_sectionattr(union adg_node* attr, struct ADGLTYPE* loc)
{
    union adg_node* node = MALLOC_AND_INIT(ADG_SECTIONATTR, loc);
    node->sectionattr.attr = attr;
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
void adg_node_append_sectionattr(union adg_node* actionattrs, union adg_node* next)
{
    union adg_node* last = actionattrs;

    assert(actionattrs->info.type == ADG_SECTIONATTR);
    assert(next->info.type == ADG_SECTIONATTR);
    assert(next->sectionattr.next == NULL);

    while (last->actionattr.next != NULL)
        last = (union adg_node*)last->actionattr.next;

    last->actionattr.next = &next->actionattr;
}

/* ------------------------------------------------------------------------- */
void adg_node_append_action(union adg_node* action, union adg_node* next)
{
    union adg_node* last = action;

    assert(adg_node_is_action(action));
    assert(adg_node_is_action(next));
    assert(next->action_base.next == NULL);

    while (last->action_base.next != NULL)
        last = (union adg_node*)last->action_base.next;

    last->action_base.next = &next->action_base;
}

/* ------------------------------------------------------------------------- */
void adg_node_append_actionattr(union adg_node* actionattrs, union adg_node* next)
{
    union adg_node* last = actionattrs;

    assert(actionattrs->info.type == ADG_ACTIONATTR);
    assert(next->info.type == ADG_ACTIONATTR);
    assert(next->actionattr.next == NULL);

    while (last->actionattr.next != NULL)
        last = (union adg_node*)last->actionattr.next;

    last->actionattr.next = &next->actionattr;
}

/* ------------------------------------------------------------------------- */
void adg_node_destroy(union adg_node* node)
{
    switch (node->info.type)
    {
        case ADG_HEADER_PREAMBLE      : free(node->header_preamble.text); break;
        case ADG_HEADER_POSTAMBLE     : free(node->header_postamble.text); break;
        case ADG_SOURCE_PREAMBLE      : free(node->source_preamble.text); break;
        case ADG_SOURCE_POSTAMBLE     : free(node->source_postamble.text); break;
        case ADG_ACTION_TABLE         : break;
        case ADG_HELP                 : free(node->help.text); break;
        case ADG_FUNC                 : free(node->func.name); break;
        case ADG_RUNAFTER             : free(node->runafter.str); break;
        case ADG_REQUIRES             : free(node->requires.str); break;
        case ADG_METADEP              : free(node->metadep.str); break;
        case ADG_ARG                  : break;
        case ADG_OPTIONAL_ARG         : break;
        case ADG_ARGNAME              : free(node->argname.str); break;
        case ADG_EXPLICIT_ACTION      : free(node->explicit_action.longopt); break;
        case ADG_IMPLICIT_ACTION      : free(node->implicit_action.name); break;
        case ADG_ACTIONATTR           : break;
        case ADG_SECTION              : free(node->section.name); break;
        case ADG_SECTIONATTR          : break;
        case ADG_EXPLICIT_META_ACTION : free(node->explicit_meta_action.longopt); break;
        case ADG_IMPLICIT_META_ACTION : free(node->implicit_meta_action.name); break;
    }

    free(node);
}

/* ------------------------------------------------------------------------- */
void adg_node_destroy_recursive(union adg_node* node)
{
    if (node->base.left)
        adg_node_destroy_recursive(node->base.left);
    if (node->base.right)
        adg_node_destroy_recursive(node->base.right);

    adg_node_destroy(node);
}

/* ------------------------------------------------------------------------- */
static void write_connections(union adg_node* node, FILE* fp)
{
    switch (node->info.type)
    {
        case ADG_HEADER_PREAMBLE :
            if (node->header_preamble.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->header_preamble.next);
            break;
        case ADG_HEADER_POSTAMBLE :
            if (node->header_postamble.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->header_postamble.next);
            break;
        case ADG_SOURCE_PREAMBLE :
            if (node->source_preamble.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->source_preamble.next);
            break;
        case ADG_SOURCE_POSTAMBLE :
            if (node->source_postamble.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->source_postamble.next);
            break;
        case ADG_ACTION_TABLE :
            if (node->action_table.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->action_table.next);
            if (node->action_table.sections) fprintf(fp, "    N%p -> N%p [label=\"sections\"];\n", node, node->action_table.sections);
            break;
        case ADG_HELP : break;
        case ADG_FUNC : break;
        case ADG_RUNAFTER :
            if (node->runafter.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->runafter.next);
            break;
        case ADG_REQUIRES :
            if (node->requires.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->requires.next);
            break;
        case ADG_METADEP :
            if (node->requires.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->metadep.next);
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
        case ADG_ACTIONATTR :
            if (node->actionattr.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->actionattr.next);
            if (node->actionattr.attr) fprintf(fp, "    N%p -> N%p [label=\"attr\"];\n", node, node->actionattr.attr);
            break;
        case ADG_SECTION :
            if (node->section.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->section.next);
            if (node->section.attrs) fprintf(fp, "    N%p -> N%p [label=\"attrs\"];\n", node, node->section.attrs);
            break;
        case ADG_SECTIONATTR :
            if (node->sectionattr.next) fprintf(fp, "    N%p -> N%p [label=\"next\"];\n", node, node->sectionattr.next);
            if (node->sectionattr.attr) fprintf(fp, "    N%p -> N%p [label=\"attr\"];\n", node, node->sectionattr.attr);
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
        case ADG_HEADER_PREAMBLE      : fprintf(fp, "hdr preamble"); break;
        case ADG_HEADER_POSTAMBLE     : fprintf(fp, "hdr postamble"); break;
        case ADG_SOURCE_PREAMBLE      : fprintf(fp, "src preamble"); break;
        case ADG_SOURCE_POSTAMBLE     : fprintf(fp, "src postamble"); break;
        case ADG_ACTION_TABLE         : fprintf(fp, "action table"); break;
        case ADG_HELP                 : fprintf(fp, "help"); break;
        case ADG_FUNC                 : fprintf(fp, "func: %s", node->func.name); break;
        case ADG_RUNAFTER             : fprintf(fp, "runafter: %s", node->runafter.str); break;
        case ADG_REQUIRES             : fprintf(fp, "runafter: %s", node->requires.str); break;
        case ADG_METADEP              : fprintf(fp, "metadep: %s", node->metadep.str); break;
        case ADG_ARG                  : fprintf(fp, "arg"); break;
        case ADG_OPTIONAL_ARG         : fprintf(fp, "optional arg: %s", node->optional_arg.continued ? "continued" : "not continued"); break;
        case ADG_ARGNAME              : fprintf(fp, "argname: %s", node->argname.str); break;
        case ADG_ACTIONATTR           : fprintf(fp, "actionattr"); break;
        case ADG_SECTION              : fprintf(fp, "section: %s", node->section.name); break;
        case ADG_SECTIONATTR          : fprintf(fp, "sectionattr"); break;
        case ADG_IMPLICIT_ACTION      : fprintf(fp, "%s", node->implicit_action.name); break;
        case ADG_IMPLICIT_META_ACTION :
            fprintf(fp, "%s[...]", node->implicit_meta_action.name);
            break;
        case ADG_EXPLICIT_ACTION :
            if (node->explicit_action.shortopt != '\0')
                fprintf(fp, "%s(%c)", node->explicit_action.longopt, node->explicit_action.shortopt);
            else
                fprintf(fp, "%s()", node->explicit_action.longopt);
            break;
        case ADG_EXPLICIT_META_ACTION :
            if (node->explicit_meta_action.shortopt != '\0')
                fprintf(fp, "%s(%c)[...]", node->explicit_meta_action.longopt, node->explicit_meta_action.shortopt);
            else
                fprintf(fp, "%s()[...]", node->explicit_meta_action.longopt);
            break;
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

/* ------------------------------------------------------------------------- */
int adg_node_is_action(const union adg_node* node)
{
    return node->info.type == ADG_EXPLICIT_ACTION
        || node->info.type == ADG_IMPLICIT_ACTION
        || node->info.type == ADG_EXPLICIT_META_ACTION
        || node->info.type == ADG_IMPLICIT_META_ACTION;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_find(union adg_node* node, enum adg_node_type type)
{
    union adg_node* found;
    if (node->info.type == type)
        return node;

    if (node->base.left)
        if ((found = adg_node_find(node->base.left, type)) != NULL)
            return found;
    if (node->base.right)
        if ((found = adg_node_find(node->base.right, type)) != NULL)
            return found;

    return NULL;
}

/* ------------------------------------------------------------------------- */
union adg_node* adg_node_find_action_matching(union adg_node* node, const char* action_node_name)
{
    union adg_node* found;
    if (adg_node_is_action(node))
    {
        const char* name;
        switch (node->info.type)
        {
            case ADG_EXPLICIT_ACTION      : name = node->explicit_action.longopt; break;
            case ADG_EXPLICIT_META_ACTION : name = node->explicit_meta_action.longopt; break;
            case ADG_IMPLICIT_ACTION      : name = node->implicit_action.name; break;
            case ADG_IMPLICIT_META_ACTION : name = node->implicit_meta_action.name; break;

            default : assert(0); return NULL;
        }
        if (strcmp(name, action_node_name) == 0)
            return node;
    }

    if (node->base.left)
        if ((found = adg_node_find_action_matching(node->base.left, action_node_name)) != NULL)
            return found;
    if (node->base.right)
        if ((found = adg_node_find_action_matching(node->base.right, action_node_name)) != NULL)
            return found;

    return NULL;
}

/* ------------------------------------------------------------------------- */
int adg_node_generate_help_action_if_not_available(union adg_node* root)
{
    union adg_node* action_table;
    union adg_node* section;
    ADGLTYPE loc = {1, 1, 1, 1};

    assert(adg_node_is_block(root));

    if (adg_node_find_action_matching(root, "help") != NULL)
        return 0;

    {
        union adg_node* func = adg_node_new_func(adg_str_dup("printHelp"), &loc);
        union adg_node* argnames = adg_node_new_argname(
            adg_node_new_argname(NULL, adg_str_dup("all"), &loc), adg_str_dup("section"), &loc);
        union adg_node* help = adg_node_new_help(adg_str_dup("Prints this help text."), &loc);
        union adg_node* attrs = adg_node_new_actionattr(help, &loc);
        union adg_node* action = adg_node_new_explicit_action(adg_str_dup("help()"), attrs, &loc);
        union adg_node* args = adg_node_new_optional_arg(NULL, argnames, 0, &loc);
        section = adg_node_new_section(action, adg_str_dup("help-action"), &loc);
        adg_node_append_actionattr(attrs, adg_node_new_actionattr(args, &loc));
        adg_node_append_actionattr(attrs, adg_node_new_actionattr(func, &loc));
    }

    action_table = adg_node_find(root, ADG_ACTION_TABLE);
    if (action_table == NULL)
    {
        action_table = adg_node_new_action_table(section, &loc);
        adg_node_append_block(root, action_table);
    }
    else
    {
        adg_node_append_section(action_table->action_table.sections, section);
    }

    return 0;
}
