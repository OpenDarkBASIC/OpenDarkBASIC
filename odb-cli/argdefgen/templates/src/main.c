#include <stdio.h>

int main(int argc, char** argv)
{
    FILE* fp;
    int i;

    if (argc < 2)
        return -1;

    fp = fopen(argv[1], "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Failed to open file `%s'\n", argv[1]);
        goto open_outfile_failed;
    }

    fprintf(fp, "#include \"argdefgen/argparse.h\"\n\n");

    for (i = 2; i + 1 < argc; )
    {
        char c;
        const char* varname = argv[i++];
        const char* srcfile = argv[i++];
        FILE* src = fopen(srcfile, "r");
        if (src == NULL)
        {
            fprintf(stderr, "Failed to open file `%s'\n", srcfile);
            goto open_srcfile_failed;
        }

        fprintf(fp, "const char* %s = \n\"", varname);
        while (1) {
            c = fgetc(src);
            if (c == EOF)
                break;

            if (c == '\n')
            {
                fprintf(fp, "\\n\"\n\"");
                continue;
            }

            if (c == '\\' || c == '"')
                fputc('\\', fp);
            fputc(c, fp);
        }

        fprintf(fp, "\";\n");
        fclose(src);
    }

    fclose(fp);
    return 0;

    open_srcfile_failed : fclose(fp);
    open_outfile_failed : return -1;
}
