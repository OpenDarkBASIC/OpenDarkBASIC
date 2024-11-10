#include "gmock/gmock.h"

extern "C" {
int         odbtests_ast = 0;
const char* odbtests_ast_filename = NULL;
}

static int
parse_cmdline(int argc, char** argv)
{
    int i;
    for (i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--ast") == 0)
        {
            odbtests_ast = 1;
            if (i + 1 < argc && argv[i + 1][0] != '-')
                odbtests_ast_filename = argv[++i];
        }
        else
        {
            fprintf(stderr, "Unknown option %s\n", argv[i]);
            return 1;
        }
    }

    return 0;
}

int
main(int argc, char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    if (parse_cmdline(argc, argv) != 0)
        return 1;
    return RUN_ALL_TESTS();
}
