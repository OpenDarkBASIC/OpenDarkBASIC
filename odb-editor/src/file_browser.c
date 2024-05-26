#include "odb-editor/file_browser.h"

#include <gtk/gtk.h>

static GListModel*
expand_node_cb(gpointer item, gpointer user_data)
{
    return NULL;
}

static GtkWidget*
create_file_list(ODBEditorFileTree* tree, ODBEditorFileBrowser* file_browser)
{
    GtkTreeListModel* model;
    GtkMultiSelection* selection_model;
    GtkWidget* column_view;
    GtkListItemFactory* item_factory;
    GtkColumnViewColumn* column;

    model = gtk_tree_list_model_new(G_LIST_MODEL(tree), FALSE, FALSE, expand_node_cb, NULL, NULL);
    gtk_tree_list_model_set_autoexpand(model, TRUE);

    selection_model = gtk_multi_selection_new(G_LIST_MODEL(model));
    column_view = gtk_column_view_new(GTK_SELECTION_MODEL(selection_model));
    /*gtk_column_view_set_show_row_separators(GTK_COLUMN_VIEW(column_view), TRUE);*/
    gtk_widget_set_vexpand(column_view, TRUE);

    g_signal_connect(column_view, "activate", G_CALLBACK(column_view_activate_cb), file_browser);
    g_signal_connect(selection_model, "selection-changed", G_CALLBACK(selection_changed_cb), file_browser);

    return column_view;
}

static GtkWidget*
create_top_widget(GtkWidget* game_list)
{
    GtkWidget* search;
    GtkWidget* games;
    GtkWidget* scroll;
    GtkWidget* vbox;
    GtkWidget* groups;
    GtkWidget* paned;

    search = gtk_entry_new();
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(search), GTK_ENTRY_ICON_PRIMARY, "edit-find-symbolic");

    scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scroll), TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), game_list);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_box_append(GTK_BOX(vbox), search);
    gtk_box_append(GTK_BOX(vbox), scroll);

    groups = gtk_button_new();

    paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_paned_set_start_child(GTK_PANED(paned), groups);
    gtk_paned_set_end_child(GTK_PANED(paned), vbox);
    gtk_paned_set_resize_start_child(GTK_PANED(paned), FALSE);
    gtk_paned_set_resize_end_child(GTK_PANED(paned), TRUE);

    gtk_paned_set_position(GTK_PANED(paned), 120);

    return paned;
}

static void
odbeditor_file_browser_init(ODBEditorFileBrowser* self)
{
    vec_init(&self->selected_game_ids, sizeof(int));
}

static void
odbeditor_file_browser_dispose(GObject* object)
{
    ODBEditorFileBrowser* self = ODBEDITOR_FILE_BROWSER(object);
    gtk_widget_unparent(self->top_widget);
    vec_deinit(&self->selected_game_ids);
    mem_track_deallocation(object);
    G_OBJECT_CLASS(odbeditor_file_browser_parent_class)->dispose(object);
}

static void
odbeditor_file_browser_class_init(ODBEditorFileBrowserClass* class)
{
    GObjectClass* object_class = G_OBJECT_CLASS(class);
    object_class->dispose = odbeditor_file_browser_dispose;
    gtk_widget_class_set_layout_manager_type(GTK_WIDGET_CLASS(class), GTK_TYPE_BIN_LAYOUT);

    file_browser_signals[SIGNAL_GAMES_SELECTED] = g_signal_new("games-selected",
        G_OBJECT_CLASS_TYPE(object_class),
        G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
        0,
        NULL, NULL,
        NULL,
        G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_INT);
}

GtkWidget*
odbeditor_file_browser_new(struct db_interface* dbi, struct db* db)
{
    GtkWidget* game_list;
    ODBEditorFileBrowser* file_browser = g_object_new(ODBEDITOR_TYPE_FILE_BROWSER, NULL);
    file_browser->tree = odbeditor_game_tree_new();
    game_list = create_game_list(file_browser->tree, file_browser);
    populate_tree_from_db(file_browser->tree, dbi, db);
    file_browser->top_widget = create_top_widget(game_list);
    gtk_widget_set_parent(file_browser->top_widget, GTK_WIDGET(file_browser));

    mem_track_allocation(file_browser);

    return GTK_WIDGET(file_browser);
}

void
odbeditor_file_browser_refresh(ODBEditorFileBrowser* self, struct db_interface* dbi, struct db* db)
{
    populate_tree_from_db(self->tree, dbi, db);
}
