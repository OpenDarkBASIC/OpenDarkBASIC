#pragma once

struct plugin_info;

struct cmd_cache
{
    char dummy;
};

void
cmd_cache_init(struct cmd_cache* cache);
void
cmd_cache_deinit(struct cmd_cache* cache);

int
cmd_cache_load(struct cmd_cache* cache);
int
cmd_cache_save(struct cmd_cache* cache);

int
cmd_cache_plugin_needs_reload(
    struct cmd_cache* cache, const struct plugin_info* plugin);

int
cmd_cache_add_plugin(struct cmd_cache* cache, const struct plugin_info* plugin);
