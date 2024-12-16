find_path (GTK4_gtk_INCLUDE_DIRS
    NAMES "gtk/gtk.h"
    PATHS
        "${GTK4_DIR}/include"
    PATH_SUFFIXES "gtk-4.0")
find_path (GTK4_glib_INCLUDE_DIR
    NAMES "glib.h"
    PATHS
        "${GTK4_DIR}/include"
    PATH_SUFFIXES "glib-2.0")
find_path (GTK4_glibconfig_INCLUDE_DIR
    NAMES "glibconfig.h"
    PATHS
        "${GTK4_DIR}/lib/"
        "/usr/lib64"
        "/usr/lib"
        "/usr/lib/x86_64-linux-gnu"
    PATH_SUFFIXES "glib-2.0/include")
set (GTK4_glib_INCLUDE_DIRS
    "${GTK4_glib_INCLUDE_DIR}"
    "${GTK4_glibconfig_INCLUDE_DIR}")
find_path (GTK4_cairo_INCLUDE_DIRS
    NAMES "cairo.h"
    PATHS
        "${GTK4_DIR}/include"
    PATH_SUFFIXES "cairo")
find_path (GTK4_pango_INCLUDE_DIRS
    NAMES "pango/pango.h"
    PATHS
        "${GTK4_DIR}/include"
    PATH_SUFFIXES "pango-1.0")
find_path (GTK4_harfbuzz_INCLUDE_DIRS
    NAMES "hb.h"
    PATHS
        "${GTK4_DIR}/include"
    PATH_SUFFIXES "harfbuzz")
find_path (GTK4_fribidi_INCLUDE_DIRS
    NAMES "fribidi.h"
    PATHS
        "${GTK4_DIR}/include"
    PATH_SUFFIXES "fribidi")
find_path (GTK4_gdk-pixbuf_INCLUDE_DIRS
    NAMES "gdk-pixbuf/gdk-pixbuf.h"
    PATHS
        "${GTK4_DIR}/include"
    PATH_SUFFIXES "gdk-pixbuf-2.0")
find_path (GTK4_graphene_INCLUDE_DIR
    NAMES "graphene.h"
    PATHS
        "${GTK4_DIR}/include"
    PATH_SUFFIXES "graphene-1.0")
find_path (GTK4_graphene-config_INCLUDE_DIR
    NAMES "graphene-config.h"
    PATHS
        "${GTK4_DIR}/lib"
        "/usr/lib64"
        "/usr/lib"
        "/usr/lib/x86_64-linux-gnu"
    PATH_SUFFIXES "graphene-1.0/include")
set (GTK4_graphene_INCLUDE_DIRS
    "${GTK4_graphene_INCLUDE_DIR}"
    "${GTK4_graphene-config_INCLUDE_DIR}")
find_path (GTK4_epoxy_INCLUDE_DIRS
    NAMES "epoxy/gl.h"
    PATHS
        "${GTK4_DIR}/include")

find_library (GTK4_gtk_LIBRARY
    NAMES "gtk-4"
    PATHS
        "${GTK4_DIR}/lib")
find_library (GTK4_glib_LIBRARY
    NAMES "glib-2.0"
    PATHS
        "${GTK4_DIR}/lib")
find_library (GTK4_gobject_LIBRARY
    NAMES "gobject-2.0"
    PATHS
        "${GTK4_DIR}/lib")
find_library (GTK4_gio_LIBRARY
    NAMES "gio-2.0"
    PATHS
        "${GTK4_DIR}/lib")
set (GTK4_glib_deps
    "${GTK4_gobject_LIBRARY}"
    "${GTK4_gio_LIBRARY}")
find_library (GTK4_cairo_LIBRARY
    NAMES "cairo"
    PATHS
        "${GTK4_DIR}/lib")
find_library (GTK4_pango_LIBRARY
    NAMES "pango-1.0"
    PATHS
        "${GTK4_DIR}/lib")
find_library (GTK4_harfbuzz_LIBRARY
    NAMES "harfbuzz"
    PATHS
        "${GTK4_DIR}/lib")
find_library (GTK4_fribidi_LIBRARY
    NAMES "fribidi"
    PATHS
        "${GTK4_DIR}/lib")
find_library (GTK4_gdk-pixbuf_LIBRARY
    NAMES "gdk_pixbuf-2.0"
    PATHS
        "${GTK4_DIR}/lib")
find_library (GTK4_graphene_LIBRARY
    NAMES "graphene-1.0"
    PATHS
        "${GTK4_DIR}/lib")
find_library (GTK4_epoxy_LIBRARY
    NAMES "epoxy"
    PATHS
        "${GTK4_DIR}/lib")

find_file (GTK4_gtk_RUNTIME
    NAMES "gtk-4-1.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_glib_RUNTIME
    NAMES "glib-2.0-0.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_gobject_RUNTIME
    NAMES "gobject-2.0-0.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_gio_RUNTIME
    NAMES "gio-2.0-0.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_gmodule_RUNTIME
    NAMES "gmodule-2.0-0.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_intl8_RUNTIME
    NAMES "intl-8.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_pangocairo_RUNTIME
    NAMES "pangocairo-1.0-0.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_fribidi_RUNTIME
    NAMES "fribidi-0.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_harfbuzz_RUNTIME
    NAMES "harfbuzz.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_cairo_RUNTIME
    NAMES "cairo-2.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_cairo-gobject_RUNTIME
    NAMES "cairo-gobject-2.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_pango_RUNTIME
    NAMES "pango-1.0-0.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_png_RUNTIME
    NAMES "png16-16.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_gdk-pixbuf_RUNTIME
    NAMES "gdk_pixbuf-2.0-0.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_epoxy_RUNTIME
    NAMES "epoxy-0.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_graphene_RUNTIME
    NAMES "graphene-1.0-0.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_pangowin32_RUNTIME
    NAMES "pangowin32-1.0-0.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_tiff_RUNTIME
    NAMES "tiff4.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_jpeg_RUNTIME
    NAMES "jpeg-8.2.2.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_cairo-script-interpreter_RUNTIME
    NAMES "cairo-script-interpreter-2.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_ffi_RUNTIME
    NAMES "ffi-7.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_z_RUNTIME
    NAMES "z.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_pcre_RUNTIME
    NAMES "pcre2-8-0.dll"
    PATHS
        "${GTK4_DIR}/bin")
find_file (GTK4_freetype_RUNTIME
    NAMES "freetype-6.dll"
    PATHS
        "${GTK4_DIR}/bin")

find_program (GTK4_glib_compile_resources_PROGRAM
    NAMES "glib-compile-resources"
    PATHS
        "${GTK4_DIR}/bin")

find_program (GTK4_glib_mkenums_PROGRAM
    NAMES "glib-mkenums")

find_program (GTK4_glib_genmarshal_PROGRAM
    NAMES "glib-genmarshal")

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
file (GLOB GTK4_RUNTIMES "${GTK4_DIR}/bin/*.dll")
set (GTK4_RUNTIMES "${GTK4_RUNTIMES}" PARENT_SCOPE)

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
    if (NOT IS_ABSOLUTE ${_output_file})
        set (_output_file "${CMAKE_CURRENT_BINARY_DIR}/${_output_file}")
    endif ()

    get_filename_component (_work_dir ${_input_file} DIRECTORY)
    get_filename_component (_out_dir ${_output_file} DIRECTORY)

    add_custom_command (
        OUTPUT ${_output_file}
        DEPENDS ${_input_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${_out_dir}
        COMMAND ${GTK4_glib_compile_resources_PROGRAM}
        ARGS --generate --target=${_output_file} ${_input_file}
        WORKING_DIRECTORY ${_work_dir}
        MAIN_DEPENDENCY ${_input_file}
        COMMENT "Compiling resource ${_input_file} -> ${_output_file}"
        VERBATIM)
endmacro ()

macro (gtk_mkenums template_file out_file)
    set (_options)
    set (_one_value_keywords
        IDENTIFIER_PREFIX
        SYMBOL_PREFIX
        DECORATOR)
    set (_multi_value_keywords
        SOURCES)
    cmake_parse_arguments (_args
        "${_options}"
        "${_one_value_keywords}"
        "${_multi_value_keywords}"
        ${ARGN})
    if (NOT ${_args_UNPARSED_ARGUMENTS} STREQUAL "")
        message (FATAL_ERROR "gtk_mkenums <template file> <output>")
    endif ()

    set (_input_files ${_args_SOURCES})
    set (_output_file ${out_file})
    if (NOT IS_ABSOLUTE ${_output_file})
        set (_output_file "${CMAKE_CURRENT_BINARY_DIR}/${_output_file}")
    endif ()
    get_filename_component (_out_dir ${_output_file} DIRECTORY)

    add_custom_command (
        OUTPUT ${_output_file}
        DEPENDS ${template_file} ${_input_files}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${_out_dir}
        COMMAND ${GTK4_glib_mkenums_PROGRAM}
        ARGS
            --identifier-prefix=${_args_IDENTIFIER_PREFIX}
            --symbol-prefix=${_args_SYMBOL_PREFIX}
            --eprod=${_args_DECORATOR}
            --template=${template_file}
            --output=${_output_file}
            ${_input_files}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating enums ${_output_file}"
        VERBATIM)
endmacro ()

macro (gtk_genmarshal in_file out_file)
    set (_options
        VALIST_MARSHALLERS)
    set (_one_value_keywords
        PREFIX)
    set (_multi_value_keywords
        SOURCES)
    cmake_parse_arguments (_args
        "${_options}"
        "${_one_value_keywords}"
        "${_multi_value_keywords}"
        ${ARGN})
    if (NOT ${_args_UNPARSED_ARGUMENTS} STREQUAL "")
        message (FATAL_ERROR "gtk_mkenums <template file> <output>")
    endif ()

    set (_input_file ${in_file})
    set (_output_file ${out_file})
    if (NOT IS_ABSOLUTE ${_input_file})
        set (_input_file "${CMAKE_CURRENT_SOURCE_DIR}/${_input_file}")
    endif ()
    if (NOT IS_ABSOLUTE ${_output_file})
        set (_output_file "${CMAKE_CURRENT_BINARY_DIR}/${_output_file}")
    endif ()

    get_filename_component (_work_dir ${_input_file} DIRECTORY)
    get_filename_component (_out_dir ${_output_file} DIRECTORY)
    get_filename_component (_ext ${_output_file} EXT)

    if (_ext STREQUAL ".c")
        set (_gen_type "--body")
    elseif (_ext STREQUAL ".h")
        set (_gen_type "--header")
    else ()
        message (FATAL_ERROR "Invalid output file extension ${_ext}")
    endif ()

    set (_gen_valist)
    if (_args_VALIST_MARSHALLERS)
        set (_gen_valist "--valist-marshallers")
    endif ()

    add_custom_command (
        OUTPUT ${_output_file}
        DEPENDS ${_input_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${_out_dir}
        COMMAND ${GTK4_glib_genmarshal_PROGRAM}
        ARGS
            ${_gen_type}
            ${_gen_valist}
            --prefix=${_args_PREFIX}
            --output=${_output_file}
            ${_input_file}
        WORKING_DIRECTORY ${_work_dir}
        MAIN_DEPENDENCY ${_input_file}
        COMMENT "Generating marshal ${_input_file} -> ${_output_file}"
        VERBATIM)
endmacro ()
