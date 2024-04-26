struct Action
{
    const char* fullOption;
    const char* argDoc;
    const union HU {
        HU();
        HU(Handler standard);
        HU(MetaHandler meta);
        Handler standard;
        MetaHandler meta;
    } handler;
    const int* runafter;
    const int* requires;
    const int* metadeps;
    const struct {
        int l;
        int h;
    } argRange;
    const int priority;
    const int sectionId;
    const char shortOption;
    const unsigned char type;
    const char* help;
};
