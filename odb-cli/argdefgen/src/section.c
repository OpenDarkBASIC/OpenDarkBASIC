#include "argdefgen/node.h"
#include "argdefgen/section.h"
#include "argdefgen/str.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
struct adg_section**
adg_section_table_new_empty(void)
{
    return calloc(1, sizeof(struct adg_section*));
}

/* ------------------------------------------------------------------------- */
int
adg_section_table_name_to_index(struct adg_section** section_table, const char* name)
{
    struct adg_section** section;
    for (section = section_table; *section; ++section)
        if (strcmp((*section)->name, name) == 0)
            return section - section_table;
    return -1;
}

/* ------------------------------------------------------------------------- */
static struct adg_section*
new_section(const char* name, const char* info)
{
    struct adg_section* section = malloc(sizeof *section);
    if (section == NULL)
        goto alloc_section_failed;

    section->name = adg_str_dup(name);
    if (section->name == NULL)
        goto alloc_name_failed;

    section->info = adg_str_dup(info);
    if (section->info == NULL)
        goto alloc_info_failed;

    return section;

    alloc_info_failed    : adg_str_free(section->name);
    alloc_name_failed    : free(section);
    alloc_section_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
static int
count_sections_in_tree(const union adg_node* node)
{
    int counter = 0;
    while (node)
    {
        counter++;
        assert(node->info.type == ADG_SECTION);
        node = (union adg_node*)node->section.next;
    }

    return counter;
}

/* ------------------------------------------------------------------------- */
struct adg_section**
adg_section_table_from_nodes(union adg_node* node)
{
    int i;
    int num_sections;
    struct adg_section** section_table;

    /* Root node must be a section. */
    if (node->info.type != ADG_SECTION)
    {
        fprintf(stderr, "Error: Expected a section\n");
        return NULL;
    }

    /* +1 is a sentinel so we know how long the list is */
    num_sections = count_sections_in_tree(node);
    section_table = malloc((num_sections + 1) * sizeof(*section_table));
    if (section_table == NULL)
        goto alloc_section_table_failed;
    memset(section_table, 0, (num_sections + 1) *  sizeof(*section_table));

    i = 0;
    while (node)
    {
        struct adg_section* section;
        union adg_node* info;

        if (adg_section_table_name_to_index(section_table, node->section.name) != -1)
        {
            fprintf(stderr, "Duplicate section `%s'\n", node->section.name);
            goto new_section_failed;
        }

        info = adg_node_find(node->section.attrs, ADG_SECTIONINFO);
        if (info == NULL)
            section = new_section(node->section.name, "");
        else
            section = new_section(node->section.name, info->sectioninfo.text);
        if (section == NULL)
            goto new_section_failed;

        section_table[i++] = section;
        assert(node->info.type == ADG_SECTION);
        node = (union adg_node*)node->section.next;
    }

    return section_table;

    new_section_failed         : adg_section_table_destroy(section_table);
    alloc_section_table_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
void
adg_section_table_destroy(struct adg_section** section_table)
{
    struct adg_section** section;
    for (section = section_table; *section; ++section)
    {
        if ((*section)->name) adg_str_free((*section)->name);
        if ((*section)->info) adg_str_free((*section)->info);
    }
    free(section_table);
}
