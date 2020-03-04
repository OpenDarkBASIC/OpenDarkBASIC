#include "odbc/ast/Node.hpp"
#include <cstdio>

namespace odbc {
namespace ast {

// ----------------------------------------------------------------------------
static void doIndent(FILE* fp, int count)
{
    while (count--)
        putc(' ', fp);
}

// ----------------------------------------------------------------------------
int dumpToJSON(FILE* fp, Node* node, int indent)
{
#define IND \
    doIndent(fp, indent);

    int children = 0;
    int nodes = 0;

    fprintf(fp, "{\n");
    IND fprintf(fp, "  \"loc\": {\n");
    IND fprintf(fp, "    \"line\": [%d, %d],\n", node->info.loc.first_line, node->info.loc.last_line);
    IND fprintf(fp, "    \"column\": [%d, %d]\n", node->info.loc.first_column, node->info.loc.last_column);
    IND fprintf(fp, "  }");
    nodes++;
    if (node->base.left)
    {
        fprintf(fp, ",\n");
        IND fprintf(fp, "  \"left\": ");
        children += dumpToJSON(fp, node->base.left, indent + 2);
        nodes++;
    }
    if (node->base.right)
    {
        fprintf(fp, ",\n");
        IND fprintf(fp, "  \"right\": ");
        children += dumpToJSON(fp, node->base.right, indent + 2);
        nodes++;
    }
    if (!node->base.left && !node->base.right)
        fprintf(fp, "\n");
    IND fprintf(fp, "}");

    if (children)
        fprintf(fp, "\n");

    return nodes;
}

}
}
