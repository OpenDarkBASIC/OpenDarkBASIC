#include "odb-sdk/init.h"
#include "odb-sdk/log.h"

#include <stdio.h>

int main(int argc, char** argv)
{
    odbsdk_init();
    odbsdk_threadlocal_init();

    FILE* fp = fopen(argv[1], "w");
    fprintf(fp, "test command%0(S)%test_command\n");
    fclose(fp);

    return 0;
}
