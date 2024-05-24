#pragma once

#include <string>
#include <vector>

struct cmd_list;
struct plugin_list;

void initCommands(void);
void deinitCommands(void);

bool loadCommands(const std::vector<std::string>& args);
bool dumpCommandsJSON(const std::vector<std::string>& args);
bool dumpCommandsINI(const std::vector<std::string> &args);
bool dumpCommandNames(const std::vector<std::string>& args);
struct plugin_list* getPluginList(void);
struct cmd_list* getCommandList();

