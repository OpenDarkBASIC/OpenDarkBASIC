find_path (GTK4_gtk_INCLUDE_DIRS
    NAMES "gtk/gtk.h"
    PATHS
        "${GTK4_ROOT}/include"
    PATH_SUFFIXES "gtk-4.0")
find_path (GTK4_glib_INCLUDE_DIR
    NAMES "glib.h"
    PATHS
        "${GTK4_ROOT}/include"
    PATH_SUFFIXES "glib-2.0")
find_path (GTK4_glibconfig_INCLUDE_DIR
    NAMES "glibconfig.h"
    PATHS
        "${GTK4_ROOT}/lib/"
        "/usr/lib64"
        "/usr/lib"
    PATH_SUFFIXES "glib-2.0/include")
set (GTK4_glib_INCLUDE_DIRS
    "${GTK4_glib_INCLUDE_DIR}"
    "${GTK4_glibconfig_INCLUDE_DIR}")
find_path (GTK4_cairo_INCLUDE_DIRS
    NAMES "cairo.h"
    PATHS
        "${GTK4_ROOT}/include"
    PATH_SUFFIXES "cairo")
find_path (GTK4_pango_INCLUDE_DIRS
    NAMES "pango/pango.h"
    PATHS
        "${GTK4_ROOT}/include"
    PATH_SUFFIXES "pango-1.0")
find_path (GTK4_harfbuzz_INCLUDE_DIRS
    NAMES "hb.h"
    PATHS
        "${GTK4_ROOT}/include"
    PATH_SUFFIXES "harfbuzz")
find_path (GTK4_gdk-pixbuf_INCLUDE_DIRS
    NAMES "gdk-pixbuf/gdk-pixbuf.h"
    PATHS
        "${GTK4_ROOT}/include"
    PATH_SUFFIXES "gdk-pixbuf-2.0")
find_path (GTK4_graphene_INCLUDE_DIR
    NAMES "graphene.h"
    PATHS
        "${GTK4_ROOT}/include"
    PATH_SUFFIXES "graphene-1.0")
find_path (GTK4_graphene-config_INCLUDE_DIR
    NAMES "graphene-config.h"
    PATHS
        "${GTK4_ROOT}/lib"
        "/usr/lib64"
        "/usr/lib"
    PATH_SUFFIXES "graphene-1.0/include")
set (GTK4_graphene_INCLUDE_DIRS
    "${GTK4_graphene_INCLUDE_DIR}"
    "${GTK4_graphene-config_INCLUDE_DIR}")
find_path (GTK4_epoxy_INCLUDE_DIRS
    NAMES "epoxy/gl.h"
    PATHS
        "${GTK4_ROOT}/include")

find_library (GTK4_gtk_LIBRARY
    NAMES "gtk-4"
    PATHS
        "${GTK4_ROOT}/lib")
find_library (GTK4_glib_LIBRARY
    NAMES "glib-2.0"
    PATHS
        "${GTK4_ROOT}/lib")
find_library (GTK4_gobject_LIBRARY
    NAMES "gobject-2.0"
    PATHS
        "${GTK4_ROOT}/lib")
find_library (GTK4_gio_LIBRARY
    NAMES "gio-2.0"
    PATHS
        "${GTK4_ROOT}/lib")
set (GTK4_glib_deps
    "${GTK4_gobject_LIBRARY}"
    "${GTK4_gio_LIBRARY}")
find_library (GTK4_cairo_LIBRARY
    NAMES "cairo"
    PATHS
        "${GTK4_ROOT}/lib")
find_library (GTK4_pango_LIBRARY
    NAMES "pango-1.0"
    PATHS
        "${GTK4_ROOT}/lib")
find_library (GTK4_harfbuzz_LIBRARY
    NAMES "harfbuzz"
    PATHS
        "${GTK4_ROOT}/lib")
find_library (GTK4_gdk-pixbuf_LIBRARY
    NAMES "gdk_pixbuf-2.0"
    PATHS
        "${GTK4_ROOT}/lib")
find_library (GTK4_graphene_LIBRARY
    NAMES "graphene-1.0"
    PATHS
        "${GTK4_ROOT}/lib")
find_library (GTK4_epoxy_LIBRARY
    NAMES "epoxy"
    PATHS
        "${GTK4_ROOT}/lib")

find_file (GTK4_gtk_RUNTIME
    NAMES "gtk-4-1.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_glib_RUNTIME
    NAMES "glib-2.0-0.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_gobject_RUNTIME
    NAMES "gobject-2.0-0.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_gio_RUNTIME
    NAMES "gio-2.0-0.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_gmodule_RUNTIME
    NAMES "gmodule-2.0-0.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_intl8_RUNTIME
    NAMES "intl-8.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_pangocairo_RUNTIME
    NAMES "pangocairo-1.0-0.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_fribidi_RUNTIME
    NAMES "fribidi-0.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_harfbuzz_RUNTIME
    NAMES "harfbuzz.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_cairo_RUNTIME
    NAMES "cairo-2.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_cairo-gobject_RUNTIME
    NAMES "cairo-gobject-2.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_pango_RUNTIME
    NAMES "pango-1.0-0.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_png_RUNTIME
    NAMES "png16-16.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_gdk-pixbuf_RUNTIME
    NAMES "gdk_pixbuf-2.0-0.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_epoxy_RUNTIME
    NAMES "epoxy-0.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_graphene_RUNTIME
    NAMES "graphene-1.0-0.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_pangowin32_RUNTIME
    NAMES "pangowin32-1.0-0.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_tiff_RUNTIME
    NAMES "tiff4.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_jpeg_RUNTIME
    NAMES "jpeg-8.2.2.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_cairo-script-interpreter_RUNTIME
    NAMES "cairo-script-interpreter-2.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_ffi_RUNTIME
    NAMES "ffi-7.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_z_RUNTIME
    NAMES "z.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_pcre_RUNTIME
    NAMES "pcre2-8-0.dll"
    PATHS
        "${GTK4_ROOT}/bin")
find_file (GTK4_freetype_RUNTIME
    NAMES "freetype-6.dll"
    PATHS
        "${GTK4_ROOT}/bin")

find_program (GTK4_glib_compile_resources_PROGRAM
    NAMES "glib-compile-resources")

set (GTK4_RUNTIMES
    ${GTK4_gtk_RUNTIME}
    ${GTK4_glib_RUNTIME}
    ${GTK4_gobject_RUNTIME}
    ${GTK4_gio_RUNTIME}
    ${GTK4_gmodule_RUNTIME}
    ${GTK4_intl8_RUNTIME}
    ${GTK4_pangocairo_RUNTIME}
    ${GTK4_fribidi_RUNTIME}
    ${GTK4_harfbuzz_RUNTIME}
    ${GTK4_cairo_RUNTIME}
    ${GTK4_cairo-gobject_RUNTIME}
    ${GTK4_png_RUNTIME}
    ${GTK4_pango_RUNTIME}
    ${GTK4_gdk-pixbuf_RUNTIME}
    ${GTK4_epoxy_RUNTIME}
    ${GTK4_graphene_RUNTIME}
    ${GTK4_pangowin32_RUNTIME}
    ${GTK4_tiff_RUNTIME}
    ${GTK4_jpeg_RUNTIME}
    ${GTK4_cairo-script-interpreter_RUNTIME}
    ${GTK4_ffi_RUNTIME}
    ${GTK4_z_RUNTIME}
    ${GTK4_pcre_RUNTIME}
    ${GTK4_freetype_RUNTIME})
file (GLOB GTK4_RUNTIMES "${GTK4_ROOT}/bin/*.dll")
set (GTK4_RUNTIMES "${GTK4_RUNTIMES}" PARENT_SCOPE)

set (GTK4_FOUND 1 PARENT_SCOPE)
foreach (_component IN LISTS GTK4_FIND_COMPONENTS)
    if (GTK4_${_component}_INCLUDE_DIRS AND GTK4_${_component}_LIBRARY)
        if (NOT TARGET GTK4::${_component})
            add_library (GTK4::${_component} UNKNOWN IMPORTED)
            set_target_properties (GTK4::${_component} PROPERTIES
                IMPORTED_LOCATION ${GTK4_${_component}_LIBRARY}
                INTERFACE_INCLUDE_DIRECTORIES "${GTK4_${_component}_INCLUDE_DIRS}"
                IMPORTED_LINK_INTERFACE_LIBRARIES "${GTK4_${_component}_deps}"
                IMPORTED_RUNTIME_ARTIFACTS "${GTK4_${_component}_RUNTIMES}")
        endif ()
        set (GTK4_${_component}_FOUND 1 PARENT_SCOPE)
    else ()
        set (GTK4_FOUND 0 PARENT_SCOPE)
        message (NOTICE "Failed to find GTK4 component ${_component}\n"
            "lib: ${GTK4_${_component}_LIBRARY}\n"
            "include: ${GTK4_${_component}_INCLUDE_DIRS}\n")
    endif ()
endforeach ()

macro (gtk_compile_resource in_file out_file)
    set (_input_file ${in_file})
    set (_output_file ${out_file})
    if (NOT IS_ABSOLUTE ${_input_file})
        set (_input_file "${CMAKE_CURRENT_SOURCE_DIR}/${_input_file}")
    endif ()
    if (NOT IS_ABSOLUTE "${_output_file}")
        set (_output_file "${CMAKE_CURRENT_BINARY_DIR}/${_output_file}")
    endif ()

    get_filename_component (_work_dir ${_input_file} DIRECTORY)
    get_filename_component (_out_dir ${_output_file} DIRECTORY)

    add_custom_command (OUTPUT ${_output_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${_out_dir}
        COMMAND ${GTK4_glib_compile_resources_PROGRAM}
        ARGS --generate --target=${_output_file} ${_input_file}
        WORKING_DIRECTORY ${_work_dir}
        MAIN_DEPENDENCY ${_input_file}
        COMMENT "Compiling resource ${_input_file}"
        VERBATIM)

    unset (_out_dir)
    unset (_work_dir)
    unset (_output_file)
    unset (_input_file)
endmacro()
