struct %prefixaction
{
    const char* full_option;
    const char* arg_doc;
    int (*handler)(char**);
    const int* runafter;
    const int* requires;
    const int* metadeps;
    const struct {
        int l;
        int h;
    } arg_range;
    const int priority;
    const int section_id;
    const char short_option;
    const unsigned char type;
    const char* help;
};
