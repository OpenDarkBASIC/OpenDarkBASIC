#pragma once

struct adg_section
{
    char* name;
    char* info;
};

union adg_node;

struct adg_section**
adg_section_table_new_empty(void);

struct adg_section**
adg_section_table_from_nodes(union adg_node* root);

void
adg_section_table_destroy(struct adg_section** section_table);

int
adg_section_table_name_to_index(struct adg_section** section_table, const char* name);
