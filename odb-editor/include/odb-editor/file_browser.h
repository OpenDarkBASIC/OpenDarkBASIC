#pragma once

#include <gtk/gtk.h>

struct db_interface;
struct db;

#define VHAPP_TYPE_GAME_BROWSER (vhapp_game_browser_get_type())
G_DECLARE_FINAL_TYPE(VhAppGameBrowser, vhapp_game_browser, VHAPP, GAME_BROWSER, GtkWidget);

GtkWidget*
vhapp_game_browser_new(struct db_interface* dbi, struct db* db);

void
vhapp_game_browser_refresh(VhAppGameBrowser* self, struct db_interface* dbi, struct db* db);
