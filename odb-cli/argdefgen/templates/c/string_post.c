/* ------------------------------------------------------------------------- */
static char** split(const char* str, char c)
{
    char** list;
    char* strloc;

    {
        const char* ptr;
        int list_count = 0;
        int str_len = 0;
        for (ptr = str; *ptr; ++ptr)
        {
            if (*ptr == c)
                list_count++;
            str_len++;
        }
        list_count++;

        list = (char**)malloc((list_count + 1) * sizeof(char*) + str_len + 1);
        if (list == NULL)
            return NULL;
        strloc = (char*)(list + list_count + 1);
        strcpy(strloc, str);
    }

    {
        char* strstart = strloc;
        char* cptr = strloc;
        char** wptr = list;
        for (; *cptr; ++cptr)
        {
            if (*cptr == c)
            {
                *wptr++ = strstart;
                *cptr = '\0';
                strstart = cptr + 1;
            }
        }
        *wptr++ = strstart;
        *wptr = NULL;
    }

    return list;
}

/* ------------------------------------------------------------------------- */
static char* new_justified_line(char** line_start, char** line_end, int width)
{
    int unjustified_len;
    int word_count;
    char** wptr;
    char* line = (char*)malloc(width + 1);
    if (line == NULL)
        return NULL;
    line[0] = '\0';

    unjustified_len = 0;
    word_count = 0;
    for (wptr = line_start; wptr != line_end; ++wptr)
    {
        if (wptr != line_start)
            strcat(line, " ");
        strcat(line, *wptr);
        unjustified_len += strlen(*wptr);
        word_count++;
    }
    unjustified_len += word_count - 1;  /* W-1 spaces */

    if (unjustified_len * 1.5 < width || word_count <= 1)
        return line;

    while (unjustified_len < width)
    {
        char* cptr;
        int insert = rand() % (word_count - 1) + 1;
        for (cptr = line; *cptr; ++cptr)
        {
            if (insert == 0)
            {
                memmove(cptr + 1, cptr, strlen(cptr) + 1);
                cptr[0] = ' ';
                unjustified_len++;
                break;
            }
            if (*cptr == ' ')
                insert--;
        }
    }

    return line;
}

/* ------------------------------------------------------------------------- */
static char** justify_wrap(const char* str, int width)
{
    int len;
    int line_count;
    char** lines;
    char** line_start;
    char** line_end;
    char** word_list = split(str, ' ');
    if (word_list == NULL)
        goto alloc_word_list_failed;

    len = 0;
    line_start = word_list;
    line_end = word_list;
    line_count = 0;
    lines = NULL;
    while (*line_end)
    {
        // No space left for another word
        if (len + strlen(*line_end) > width)
        {
            void* new_lines = realloc(lines, (line_count + 2) * sizeof(char*));
            if (new_lines == NULL)
                goto realloc_lines_failed;
            lines = (char**)new_lines;

            lines[line_count] = new_justified_line(line_start, line_end, width);
            if (lines[line_count] == NULL)
                goto realloc_lines_failed;
            line_count++;

            line_start = line_end;
            len = 0;
        }
        else
        {
            len += strlen(*line_end) + 1;  // +1 = space
            line_end++;
        }
    }

    if (line_start != line_end)
    {
        void* new_lines = realloc(lines, (line_count + 2) * sizeof(char*));
        if (new_lines == NULL)
            goto realloc_lines_failed;
        lines = (char**)new_lines;

        lines[line_count] = new_justified_line(line_start, line_end, width);
        if (lines[line_count] == NULL)
            goto realloc_lines_failed;
        line_count++;
    }

    lines[line_count] = NULL;
    free(word_list);
    return lines;

    realloc_lines_failed   : while (line_count--)
                                 free(lines[line_count]);
                             if (lines)
                                 free(lines);
                             free(word_list);
    alloc_word_list_failed : return NULL;
}
