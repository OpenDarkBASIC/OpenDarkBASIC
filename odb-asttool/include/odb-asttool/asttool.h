#pragma once

enum export_type
{
    EXPORT_GRAPHVIZ = 0,
    EXPORT_GTEST = 1,
};

struct cfg
{
    const char*      input_fname;
    const char*      output_fname;
    enum export_type export_type : 2;
    unsigned         with_scopes : 1;
    unsigned         with_types : 1;
    unsigned        with_node_asserts : 1;
};
