project (gtksourceview
    LANGUAGES C
    VERSION 5.9.0)

find_package(LibXml2 REQUIRED)
find_package (Freetype REQUIRED)
find_package (GTK4 REQUIRED COMPONENTS
    gtk glib cairo pango harfbuzz fribidi gdk-pixbuf graphene)

set (gtksourceview_ROOT "${PROJECT_SOURCE_DIR}/../gtksourceview-${PROJECT_VERSION}")

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
    "${gtksourceview_ROOT}/gtksourceview/gtksourceversion.h.in"
    "${PROJECT_BINARY_DIR}/gtksourceview/gtksourceversion.h")

set (gtksourceview_CORE_PUBLIC_H
    "${gtksourceview_ROOT}/gtksourceview/gtksource.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcebuffer.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcecompletion.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcecompletioncell.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcecompletioncontext.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcecompletionprovider.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcecompletionproposal.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceencoding.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcefile.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcefileloader.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcefilesaver.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcegutter.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcegutterrenderer.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcegutterrendererpixbuf.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcegutterrenderertext.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcehover.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcehovercontext.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcehoverprovider.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcehoverdisplay.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceindenter.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceinit.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcelanguage.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcelanguagemanager.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcegutterlines.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcemap.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcemark.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcemarkattributes.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceprintcompositor.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceregion.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcescheduler.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcesearchcontext.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcesearchsettings.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcesnippet.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcesnippetchunk.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcesnippetcontext.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcesnippetmanager.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcespacedrawer.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcestyle.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcestylescheme.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcestyleschemechooser.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcestyleschemechooserbutton.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcestyleschemechooserwidget.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcestyleschememanager.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcestyleschemepreview.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcetag.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcetypes.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceutils.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceview.h"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcevimimcontext.h")
set (gtksourceview_CORE_PUBLIC_C
    "${gtksourceview_ROOT}/gtksourceview/gtksourcebuffer.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcecompletion.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcecompletioncell.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcecompletioncontext.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcecompletionprovider.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcecompletionproposal.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceencoding.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcefile.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcefileloader.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcefilesaver.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcegutter.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcegutterrenderer.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcegutterrendererpixbuf.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcegutterrenderertext.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcehover.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcehovercontext.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcehoverdisplay.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcehoverprovider.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceindenter.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceinit.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcelanguage.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcelanguagemanager.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcegutterlines.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcemap.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcemark.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcemarkattributes.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceprintcompositor.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceregion.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcescheduler.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcesearchcontext.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcesearchsettings.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcesnippet.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcesnippetchunk.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcesnippetcontext.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcesnippetmanager.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcespacedrawer.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcestyle.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcestylescheme.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcestyleschemechooser.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcestyleschemechooserbutton.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcestyleschemechooserwidget.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcestyleschememanager.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcestyleschemepreview.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcetag.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceutils.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceversion.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceview.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcevimimcontext.c")
set (gtksourceview_CORE_PRIVATE_C
    "${gtksourceview_ROOT}/gtksourceview/gtksourceassistant.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceassistantchild.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcebufferinputstream.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcebufferinternal.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcebufferoutputstream.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcecompletioninfo.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcecompletionlist.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcecompletionlistbox.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcecompletionlistboxrow.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcecontextengine.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceengine.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcegutterrendererlines.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcegutterrenderermarks.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcehoverassistant.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceinformative.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceiter.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcelanguage-parser-2.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcemarkssequence.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcepixbufhelper.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceregex.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceview-assistants.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcesnippetbundle.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourcesnippetbundle-parser.c"
    "${gtksourceview_ROOT}/gtksourceview/gtksourceview-snippets.c"
    "${gtksourceview_ROOT}/gtksourceview/implregex.c")

set (vim_SOURCES
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourcevim.c"
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourcevimcharpending.c"
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourcevimcommandbar.c"
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourcevimcommand.c"
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourceviminsert.c"
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourceviminsertliteral.c"
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourcevimjumplist.c"
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourcevimmarks.c"
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourcevimmotion.c"
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourcevimnormal.c"
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourcevimregisters.c"
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourcevimreplace.c"
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourcevimstate.c"
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourcevimtexthistory.c"
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourcevimtextobject.c"
  "${gtksourceview_ROOT}/gtksourceview/vim/gtksourcevimvisual.c")

set (completionsnippets_PUBLIC_H
    "${gtksourceview_ROOT}/gtksourceview/completion-providers/snippets/gtksourcecompletionsnippets.h")
set (completionsnippets_SOURCES
    "${gtksourceview_ROOT}/gtksourceview/completion-providers/snippets/gtksourcecompletionsnippets.c"
    "${gtksourceview_ROOT}/gtksourceview/completion-providers/snippets/gtksourcecompletionsnippetsproposal.c")

set (completionwords_PUBLIC_H
    "${gtksourceview_ROOT}/gtksourceview/completion-providers/words/gtksourcecompletionwords.h")
set (completionwords_SOURCES
    "${gtksourceview_ROOT}/gtksourceview/completion-providers/words/gtksourcecompletionwords.c"
    "${gtksourceview_ROOT}/gtksourceview/completion-providers/words/gtksourcecompletionwordsbuffer.c"
    "${gtksourceview_ROOT}/gtksourceview/completion-providers/words/gtksourcecompletionwordslibrary.c"
    "${gtksourceview_ROOT}/gtksourceview/completion-providers/words/gtksourcecompletionwordsmodel.c"
    "${gtksourceview_ROOT}/gtksourceview/completion-providers/words/gtksourcecompletionwordsproposal.c"
    "${gtksourceview_ROOT}/gtksourceview/completion-providers/words/gtksourcecompletionwordsutils.c")

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
    "${gtksourceview_ROOT}/gtksourceview/gtksourceview.gresource.xml"
    "gtksourceview-gresources.c")
gtk_compile_resource (
    "${gtksourceview_ROOT}/gtksourceview/gtksourceview.gresource.xml"
    "gtksourceview/gtksourceview-gresources.h")

gtk_genmarshal(
    "${gtksourceview_ROOT}/gtksourceview/gtksourcemarshalers.list"
    "gtksource-marshal.c"
    PREFIX "_gtk_source_marshal"
    VALIST_MARSHALLERS)
gtk_genmarshal(
    "${gtksourceview_ROOT}/gtksourceview/gtksourcemarshalers.list"
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
        $<BUILD_INTERFACE:${gtksourceview_ROOT}>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/gtksourceview>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
    PRIVATE
        $<BUILD_INTERFACE:${gtksourceview_ROOT}/gtksourceview>)
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

