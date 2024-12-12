project (gtksourceview
    VERSION 5.9.0
    LANGUAGES C)

find_package (Freetype REQUIRED)
find_package (GTK4 REQUIRED COMPONENTS
    gtk glib cairo pango harfbuzz fribidi gdk-pixbuf graphene)

set (gtksourceview_DIR "${PROJECT_SOURCE_DIR}/../gtksourceview-${PROJECT_VERSION}")

# Most of these settings are probably wrong. Just copied them from what meson
# does
set (gtksourceview_DATADIR "/usr/local/share")
if (${CMAKE_BUILD_TYPE} MATCHES Debug)
    set (gtksourceview_DEVELOPMENT_BUILD "1")
else ()
    set (gtksourceview_DEVELOPMENT_BUILD "0")
endif ()
set (gtksourceview_ENABLE_FONT_CONFIG "1")
set (gtksourceview_GETTEXT_PACKAGE "gtksourceview-${PROJECT_VERSION_MAJOR}")
set (gtksourceview_GSV_API_VERSION "${PROJECT_VERSION_MAJOR}")
set (gtksourceview_HAVE_MEMALIGN "1")
set (gtksourceview_HAVE_UNISTD_H "1")
set (gtksourceview_HICOLORDIR "/usr/local/share/icons/hicolor")
set (gtksourceview_PACKAGE_DATADIR "/usr/local/share/gtksourceview-${PROJECT_VERSION_MAJOR}")
set (gtksourceview_PACKAGE_VERSION "${PROJECT_VERSION}")
set (gtksourceview__GTK_SOURCE_EXTERN "__attribute__((visibility(\"default\"))) extern")

set (GTK_SOURCE_MAJOR_VERSION "${PROJECT_VERSION_MAJOR}")
set (GTK_SOURCE_MINOR_VERSION "${PROJECT_VERSION_MINOR}")
set (GTK_SOURCE_MICRO_VERSION "${PROJECT_VERSION_PATCH}")

configure_file ("config.h.in" "config.h")
configure_File (
    "${gtksourceview_DIR}/gtksourceview/gtksourceversion.h.in"
    "${PROJECT_BINARY_DIR}/gtksourceview/gtksourceversion.h")

set (gtksourceview_CORE_PUBLIC_H
    "${gtksourceview_DIR}/gtksourceview/gtksource.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcebuffer.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcecompletion.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcecompletioncell.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcecompletioncontext.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcecompletionprovider.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcecompletionproposal.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourceencoding.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcefile.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcefileloader.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcefilesaver.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcegutter.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcegutterrenderer.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcegutterrendererpixbuf.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcegutterrenderertext.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcehover.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcehovercontext.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcehoverprovider.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcehoverdisplay.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourceindenter.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourceinit.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcelanguage.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcelanguagemanager.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcegutterlines.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcemap.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcemark.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcemarkattributes.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourceprintcompositor.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourceregion.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcescheduler.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcesearchcontext.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcesearchsettings.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcesnippet.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcesnippetchunk.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcesnippetcontext.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcesnippetmanager.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcespacedrawer.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcestyle.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcestylescheme.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcestyleschemechooser.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcestyleschemechooserbutton.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcestyleschemechooserwidget.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcestyleschememanager.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcestyleschemepreview.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcetag.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcetypes.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourceutils.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourceview.h"
    "${gtksourceview_DIR}/gtksourceview/gtksourcevimimcontext.h")
set (gtksourceview_CORE_PUBLIC_C
    "${gtksourceview_DIR}/gtksourceview/gtksourcebuffer.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcecompletion.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcecompletioncell.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcecompletioncontext.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcecompletionprovider.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcecompletionproposal.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourceencoding.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcefile.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcefileloader.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcefilesaver.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcegutter.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcegutterrenderer.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcegutterrendererpixbuf.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcegutterrenderertext.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcehover.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcehovercontext.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcehoverdisplay.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcehoverprovider.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourceindenter.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourceinit.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcelanguage.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcelanguagemanager.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcegutterlines.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcemap.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcemark.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcemarkattributes.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourceprintcompositor.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourceregion.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcescheduler.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcesearchcontext.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcesearchsettings.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcesnippet.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcesnippetchunk.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcesnippetcontext.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcesnippetmanager.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcespacedrawer.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcestyle.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcestylescheme.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcestyleschemechooser.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcestyleschemechooserbutton.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcestyleschemechooserwidget.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcestyleschememanager.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcestyleschemepreview.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcetag.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourceutils.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourceversion.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourceview.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcevimimcontext.c")
set (gtksourceview_CORE_PRIVATE_C
    "${gtksourceview_DIR}/gtksourceview/gtksourceassistant.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourceassistantchild.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcebufferinputstream.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcebufferinternal.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcebufferoutputstream.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcecompletioninfo.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcecompletionlist.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcecompletionlistbox.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcecompletionlistboxrow.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcecontextengine.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourceengine.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcegutterrendererlines.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcegutterrenderermarks.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcehoverassistant.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourceinformative.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourceiter.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcelanguage-parser-2.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcemarkssequence.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcepixbufhelper.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourceregex.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourceview-assistants.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcesnippetbundle.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourcesnippetbundle-parser.c"
    "${gtksourceview_DIR}/gtksourceview/gtksourceview-snippets.c"
    "${gtksourceview_DIR}/gtksourceview/implregex.c")

set (vim_SOURCES
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourcevim.c"
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourcevimcharpending.c"
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourcevimcommandbar.c"
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourcevimcommand.c"
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourceviminsert.c"
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourceviminsertliteral.c"
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourcevimjumplist.c"
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourcevimmarks.c"
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourcevimmotion.c"
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourcevimnormal.c"
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourcevimregisters.c"
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourcevimreplace.c"
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourcevimstate.c"
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourcevimtexthistory.c"
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourcevimtextobject.c"
  "${gtksourceview_DIR}/gtksourceview/vim/gtksourcevimvisual.c")

set (completionsnippets_PUBLIC_H
    "${gtksourceview_DIR}/gtksourceview/completion-providers/snippets/gtksourcecompletionsnippets.h")
set (completionsnippets_SOURCES
    "${gtksourceview_DIR}/gtksourceview/completion-providers/snippets/gtksourcecompletionsnippets.c"
    "${gtksourceview_DIR}/gtksourceview/completion-providers/snippets/gtksourcecompletionsnippetsproposal.c")

set (completionwords_PUBLIC_H
    "${gtksourceview_DIR}/gtksourceview/completion-providers/words/gtksourcecompletionwords.h")
set (completionwords_SOURCES
    "${gtksourceview_DIR}/gtksourceview/completion-providers/words/gtksourcecompletionwords.c"
    "${gtksourceview_DIR}/gtksourceview/completion-providers/words/gtksourcecompletionwordsbuffer.c"
    "${gtksourceview_DIR}/gtksourceview/completion-providers/words/gtksourcecompletionwordslibrary.c"
    "${gtksourceview_DIR}/gtksourceview/completion-providers/words/gtksourcecompletionwordsmodel.c"
    "${gtksourceview_DIR}/gtksourceview/completion-providers/words/gtksourcecompletionwordsproposal.c"
    "${gtksourceview_DIR}/gtksourceview/completion-providers/words/gtksourcecompletionwordsutils.c")

gtk_mkenums (
    "gtksource-enumtypes.h.in"
    "gtksourceview/gtksource-enumtypes.h"
    SOURCES ${gtksourceview_CORE_PUBLIC_H}
    IDENTIFIER_PREFIX GtkSource
    SYMBOL_PREFIX GTK_SOURCE
    DECORATOR GTK_SOURCE_ENUM_EXTERN)
gtk_mkenums (
    "gtksource-enumtypes.c.in"
    "gtksource-enumtypes.c"
    SOURCES ${gtksourceview_CORE_PUBLIC_H}
    IDENTIFIER_PREFIX GtkSource
    SYMBOL_PREFIX GTK_SOURCE)

gtk_compile_resource (
    "${gtksourceview_DIR}/gtksourceview/gtksourceview.gresource.xml"
    "gtksourceview-gresources.c")
gtk_compile_resource (
    "${gtksourceview_DIR}/gtksourceview/gtksourceview.gresource.xml"
    "gtksourceview/gtksourceview-gresources.h")

gtk_genmarshal(
    "${gtksourceview_DIR}/gtksourceview/gtksourcemarshalers.list"
    "gtksource-marshal.c"
    PREFIX "_gtk_source_marshal"
    VALIST_MARSHALLERS)
gtk_genmarshal(
    "${gtksourceview_DIR}/gtksourceview/gtksourcemarshalers.list"
    "gtksourceview/gtksource-marshal.h"
    PREFIX "_gtk_source_marshal"
    VALIST_MARSHALLERS)

add_library (gtksourceview SHARED
    "${PROJECT_BINARY_DIR}/gtksourceview/gtksource-enumtypes.h"
    "${PROJECT_BINARY_DIR}/gtksourceview/gtksource-marshal.h"
    "${PROJECT_BINARY_DIR}/gtksourceview/gtksourceview-gresources.h"
    "${PROJECT_BINARY_DIR}/gtksource-enumtypes.c"
    "${PROJECT_BINARY_DIR}/gtksource-marshal.c"
    "${PROJECT_BINARY_DIR}/gtksourceview-gresources.c"
    ${gtksourceview_CORE_PUBLIC_H}
    ${gtksourceview_CORE_PUBLIC_C}
    ${gtksourceview_CORE_PRIVATE_C}
    ${vim_SOURCES}
    ${completionsnippets_SOURCES}
    ${completionwords_SOURCES})
target_include_directories (gtksourceview
    PUBLIC
        $<BUILD_INTERFACE:${gtksourceview_DIR}>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/gtksourceview>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
    PRIVATE
        $<BUILD_INTERFACE:${gtksourceview_DIR}/gtksourceview>)
target_compile_definitions (gtksourceview
    PRIVATE
        GTK_SOURCE_COMPILATION
        G_LOG_DOMAIN="GtkSourceView")
target_compile_options (gtksourceview
    PRIVATE
        $<$<C_COMPILER_ID:GNU>:-Wno-deprecated-declarations>)
target_link_libraries (gtksourceview
    PRIVATE
        GTK4::glib
        GTK4::cairo
        GTK4::pango
        GTK4::harfbuzz
        GTK4::fribidi
        GTK4::gdk-pixbuf
        GTK4::graphene
        GTK4::gtk
        Freetype::Freetype
        LibXml2::LibXml2)

