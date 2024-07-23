#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int
main(int argc, char** argv)
{
    if (argc == 1)
    {
        char buf[256];
        int  len = fread(buf, 1, 256, stdin);
        if (len > 0)
            fwrite(buf, 1, len, stdout);
    }
    else if (strcmp(argv[1], "--stderr") == 0)
    {
        char buf[256];
        int  len = fread(buf, 1, 256, stdin);
        if (len > 0)
            fwrite(buf, 1, len, stderr);
    }
    else if (strcmp(argv[1], "--exit") == 0)
        return atoi(argv[2]);

    return 0;
}
