#include "odb-editor/file_browser.h"
#include "odb-util/init.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>

#define ODBEDITOR_TYPE_PLUGIN_MODULE (odbeditor_plugin_module_get_type())
G_DECLARE_FINAL_TYPE(
    ODBEditorPluginModule,
    odbeditor_plugin_module,
    ODBEDITOR,
    PLUGIN_MODULE,
    GTypeModule)

struct _ODBEditorPluginModule
{
    GTypeModule parent_instance;
};
struct _ODBEditorPluginModuleClass
{
    GTypeModuleClass parent_class;
};
G_DEFINE_TYPE(
    ODBEditorPluginModule, odbeditor_plugin_module, G_TYPE_TYPE_MODULE)

static gboolean
odbeditor_plugin_module_load(GTypeModule* type_module)
{
    log_dbg("odbeditor_plugin_module_load()\n");
    mem_track_allocation(type_module);
    return TRUE;
}

static void
odbeditor_plugin_module_unload(GTypeModule* type_module)
{
    log_dbg("odbeditor_plugin_module_unload()\n");
    mem_track_deallocation(type_module);
}

static void
odbeditor_plugin_module_init(ODBEditorPluginModule* self)
{
}

static void
odbeditor_plugin_module_class_init(ODBEditorPluginModuleClass* class)
{
    GTypeModuleClass* module_class = G_TYPE_MODULE_CLASS(class);

    module_class->load = odbeditor_plugin_module_load;
    module_class->unload = odbeditor_plugin_module_unload;
}

struct plugin
{
    // struct plugin_lib lib;
    // struct plugin_ctx* ctx;
    GTypeModule* plugin_module;
    GtkWidget*   ui_center;
    GtkWidget*   ui_pane;
};

static void
page_removed(
    GtkNotebook* self, GtkWidget* child, guint page_num, gpointer user_data)
{
    log_dbg("page_removed()\n");
}

static GtkWidget*
property_panel_new(void)
{
    GtkWidget* notebook = gtk_notebook_new();
    g_signal_connect(notebook, "page-removed", G_CALLBACK(page_removed), NULL);
    return notebook;
}

static GtkWidget*
plugin_view_new(void)
{
    // GtkWidget* notebook = gtk_notebook_new();
    // g_signal_connect(notebook, "page-removed", G_CALLBACK(page_removed),
    // NULL); return notebook;
    GtkTextTagTable* tag_table = gtk_text_tag_table_new();
    GtkSourceBuffer* buffer = gtk_source_buffer_new(tag_table);
    gtk_text_buffer_set_text(
        GTK_TEXT_BUFFER(buffer), "for n = 1 to 10\n    print n\nnext n\n", -1);

    GtkTextTag* keyword = gtk_source_buffer_create_source_tag(
        buffer, "keyword", "foreground", "red", NULL);
    GtkTextTag* number = gtk_source_buffer_create_source_tag(
        buffer, "number", "foreground", "blue", NULL);
    GtkTextTag* operator = gtk_source_buffer_create_source_tag(
        buffer, "operator", "foreground", "cyan", NULL);

    GtkTextIter start, end;
    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &start, 0);
    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &end, 3);
    gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(buffer), keyword, &start, &end);

    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &start, 6);
    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &end, 7);
    gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(buffer), operator, &start, &end);

    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &start, 8);
    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &end, 9);
    gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(buffer), number, &start, &end);

    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &start, 10);
    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &end, 12);
    gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(buffer), keyword, &start, &end);

    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &start, 14);
    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &end, 16);
    gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(buffer), number, &start, &end);

    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &start, 20);
    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &end, 25);
    gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(buffer), keyword, &start, &end);

    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &start, 28);
    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &end, 32);
    gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(buffer), keyword, &start, &end);

    GtkWidget* editor = gtk_source_view_new_with_buffer(buffer);
    gtk_source_view_set_show_line_marks(GTK_SOURCE_VIEW(editor), TRUE);
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(editor), TRUE);
    gtk_source_view_set_show_right_margin(GTK_SOURCE_VIEW(editor), TRUE);
    return editor;
}

static gboolean
shortcut_activated(GtkWidget* widget, GVariant* unused, gpointer user_data)
{
    log_dbg("activated shift+r\n");
    return TRUE;
}

static void
setup_global_shortcuts(GtkWidget* window)
{
    GtkEventController* controller;
    GtkShortcutTrigger* trigger;
    GtkShortcutAction*  action;
    GtkShortcut*        shortcut;

    controller = gtk_shortcut_controller_new();
    gtk_shortcut_controller_set_scope(
        GTK_SHORTCUT_CONTROLLER(controller), GTK_SHORTCUT_SCOPE_GLOBAL);
    gtk_widget_add_controller(window, controller);

    trigger = gtk_keyval_trigger_new(GDK_KEY_r, GDK_SHIFT_MASK);
    action = gtk_callback_action_new(shortcut_activated, NULL, NULL);
    shortcut = gtk_shortcut_new(trigger, action);
    gtk_shortcut_controller_add_shortcut(
        GTK_SHORTCUT_CONTROLLER(controller), shortcut);
}

static void
activate(GtkApplication* app, gpointer user_data)
{
    GtkWidget* window;
    GtkWidget* file_browser;
    GtkWidget* paned1;
    GtkWidget* paned2;
    GtkWidget* plugin_view;
    GtkWidget* property_panel;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "OpenDarkBASIC Editor");
    gtk_window_set_default_size(GTK_WINDOW(window), 1280, 720);
    setup_global_shortcuts(window);

    plugin_view = plugin_view_new();
    property_panel = property_panel_new();

    // file_browser = odbeditor_file_browser_new(ctx->dbi, ctx->db);
    // g_signal_connect(file_browser, "games-selected",
    // G_CALLBACK(on_games_selected), ctx);
    file_browser = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_paned_set_position(GTK_PANED(file_browser), 120);

    paned2 = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_start_child(GTK_PANED(paned2), plugin_view);
    gtk_paned_set_end_child(GTK_PANED(paned2), property_panel);
    gtk_paned_set_resize_start_child(GTK_PANED(paned2), TRUE);
    gtk_paned_set_resize_end_child(GTK_PANED(paned2), FALSE);
    // gtk_paned_set_position(GTK_PANED(paned2), 800);

    paned1 = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_start_child(GTK_PANED(paned1), file_browser);
    gtk_paned_set_end_child(GTK_PANED(paned1), paned2);
    gtk_paned_set_resize_start_child(GTK_PANED(paned1), FALSE);
    gtk_paned_set_resize_end_child(GTK_PANED(paned1), TRUE);
    gtk_paned_set_position(GTK_PANED(paned1), 600);

    gtk_window_set_child(GTK_WINDOW(window), paned1);
    gtk_window_maximize(GTK_WINDOW(window));
    gtk_widget_set_visible(window, 1);

    /*open_plugin(GTK_NOTEBOOK(plugin_view), GTK_NOTEBOOK(property_panel),
            &ctx->plugins, ctx->dbi, ctx->db, cstr_view("AI Tool"));*/
    // open_plugin(GTK_NOTEBOOK(plugin_view), GTK_NOTEBOOK(property_panel),
    //         &ctx->plugins, ctx->dbi, ctx->db, cstr_view("VOD Review"));
    // open_plugin(GTK_NOTEBOOK(plugin_view), GTK_NOTEBOOK(property_panel),
    //         &ctx->plugins, ctx->dbi, ctx->db, cstr_view("Search"));
}

int
main(int argc, char** argv)
{
    GtkApplication* app;
    int             status;

    odbutil_init();

    app = gtk_application_new(
        "com.github.opendarkbasic.odb-editor", G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    odbutil_deinit();

    return status;
}
