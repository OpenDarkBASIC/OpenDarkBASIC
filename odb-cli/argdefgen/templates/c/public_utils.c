/* ------------------------------------------------------------------------- */
int %prefixaction_id(const char* full_option)
{
    const struct %prefixaction* action;
    for (action = g_actions; ACTION_VALID(action); ++action)
        if (strcmp(action->full_option, full_option) == 0)
            return action - g_actions;
    return -1;
}
