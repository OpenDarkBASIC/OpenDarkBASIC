#include <unistd.h>

int
main(int argc, char** argv)
{
    char buf[256];
    int  len = read(STDIN_FILENO, buf, 256);
    if (len > 0)
        write(STDOUT_FILENO, buf, len);
    return 0;
}
