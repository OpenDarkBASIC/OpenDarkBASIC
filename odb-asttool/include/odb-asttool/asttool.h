#pragma once

enum export_type
{
    EXPORT_GRAPHVIZ = 0,
    EXPORT_GTEST = 1,
};

enum style_type
{
    STYLE_CATPUCCIN = 0,
    STYLE_NIGHTFLY = 1,
};

struct cfg
{
    const char*      input_fname;
    const char*      output_fname;
    const char*      node_filter;
    enum export_type export_type : 2;
    enum style_type  style : 2;
    unsigned         with_scopes : 1;
    unsigned         with_types : 1;
    unsigned         with_node_types : 1;
    unsigned         with_node_properties : 1;
};
