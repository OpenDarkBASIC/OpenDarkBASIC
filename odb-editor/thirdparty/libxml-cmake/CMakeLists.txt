project (libxml
    VERSION 2.13.5
    LANGUAGES C)

find_package (GTK4 REQUIRED COMPONENTS
    glib)

set (libxml_DIR "${PROJECT_SOURCE_DIR}/../libxml-${PROJECT_VERSION}")

set (libxml_LIB_TYPE "STATIC" CACHE STRING "SHARED or STATIC library")
set_property (CACHE libxml_LIB_TYPE PROPERTY STRINGS "STATIC;SHARED")

option (libxml_C14N "Canonical XML 1.0 support" ON)
option (libxml_CATALOG "XML Catalogs support" ON)
option (libxml_DEBUG "Debugging module and shell" OFF)
option (libxml_FTP "FTP support" OFF)
option (libxml_HISTORY "History support for shell" OFF)
option (libxml_HTML "HTML parser" ON)
option (libxml_HTTP "HTTP support" OFF)
option (libxml_ICONV "iconv support" ON)  # TODO meson custom dependency
option (libxml_ICU "ICU support" ON)
option (libxml_IPV6 "Compilation of IPv6 code" ON)
option (libxml_ISO8859X "ISO-8859-X support if no iconv" ON)
option (libxml_LEGACY "Maximum ABI compatibility" OFF)
option (libxml_LZMA "LZMA support" OFF)
option (libxml_MODULES "Dynamic modules support" OFF)
option (libxml_OUTPUT "Serialization support" ON)
option (libxml_PATTERN "xmlPattern selection interface" ON)
option (libxml_PUSH "push parser interfaces" ON)
option (libxml_PYTHON "Python bindings" OFF)
option (libxml_READER "xmlReader parsing interface" ON)
option (libxml_READLINE "use readline in DIR (for shell history)" OFF)
option (libxml_REGEXPS "Regular expressions support" ON)
option (libxml_SAX1 "Older SAX1 interface" ON)
option (libxml_SCHEMAS "XML Schemas 1.0 and RELAX NG support" ON)
option (libxml_SCHEMATRON "Schematron support" ON)
option (libxml_THREADS "Multithreading support" ON)
option (libxml_THREAD_ALLOC "per-thread malloc hooks" OFF)
option (libxml_TLS "thread-local storage" OFF)
option (libxml_TREE "DOM like tree manipulation APIs" ON)
option (libxml_VALID "DTD validation support" ON)
option (libxml_WRITER "xmlWriter serialization interface" ON)
option (libxml_XINCLUDE "XInclude 1.0 support" ON)
option (libxml_XPATH "XPath 1.0 support" ON)
option (libxml_XPTR "XPointer support" ON)
option (libxml_XPTR_LOCS "XPointer ranges and points" OFF)
option (libxml_ZLIB "ZLIB support" ON)
option (libxml_MINIMUM "build a minimally sized library (default=OFF)" OFF)

include (CheckIncludeFile)
include (CheckCSourceCompiles)
macro (check_func_attribute in_attr out_attr have_attr)
    check_c_source_compiles ("int main(void) { ${in_attr} int value; return 0; }" ${have_attr})
    if (${have_attr})
        set (${out_attr} ${in_attr})
        set (${have_attr} 1)
    else ()
        set (${out_attr} "")
        set (${have_attr} 0)
    endif ()
endmacro ()

check_include_file (stdint.h HAVE_STDINT_H)
check_include_file (fcntl.h HAVE_FCNTL_H)
check_include_file (unistd.h HAVE_UNISTD_H)
check_include_file (sys/stat.h HAVE_SYS_STAT_H)
check_include_file (sys/mman.h HAVE_SYS_MMAN_H)
check_include_file (sys/socket.h HAVE_SYS_SOCKET_H)
check_include_file (netinet/in.h HAVE_NETINET_IN_H)
check_include_file (arpa/inet.h HAVE_ARPA_INET_H)
check_include_file (netdb.h HAVE_NETDB_H)
check_include_file (sys/select.h HAVE_SYS_SELECT_H)
check_include_file (poll.h HAVE_POLL_H)
check_include_file (sys/time.h HAVE_SYS_TIME_H)
check_include_file (sys/timeb.h HAVE_SYS_TIMEB_H)
check_include_file (dl.h HAVE_DL_H)
check_include_file (dlfcn.h HAVE_DLFCN_H)
check_include_file (glob.h HAVE_GLOB_H)
check_func_attribute ("__attribute__((__destructor__))" ATTRIBUTE_DESTRUCTOR HAVE_ATTRIBUTE_DESTRUCTOR)
configure_file ("config.h.in" "include/config.h")

macro (set01 out_var in_var)
    if (${in_var})
        set (${out_var} 1)
    else ()
        set (${out_var} 0)
    endif ()
endmacro ()

include (CheckFunctionExists)
check_function_exists (gettimeofday HAVE_GETTIMEOFDAY)
check_function_exists (ftime HAVE_FTIME)
check_function_exists (stat HAVE_STAT)
check_function_exists (mmap HAVE_MMAP)
check_function_exists (munmap HAVE_MUNMAP)
set (VERSION "${PROJECT_VERSION}")
set (LIBXML_VERSION_NUMBER "${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}${PROJECT_VERSION_PATCH}")
set01 (WITH_THREADS ${libxml_THREADS})
set01 (WITH_THREAD_ALLOC ${libxml_THREAD_ALLOC})
set01 (WITH_TREE ${libxml_TREE})
set01 (WITH_OUTPUT ${libxml_OUTPUT})
set01 (WITH_PUSH ${libxml_PUSH})
set01 (WITH_READER ${libxml_READER})
set01 (WITH_PATTERN ${libxml_PATTERN})
set01 (WITH_WRITER ${libxml_WRITER})
set01 (WITH_SAX1 ${libxml_SAX1})
set01 (WITH_FTP ${libxml_FTP})
set01 (WITH_HTTP ${libxml_HTTP})
set01 (WITH_VALID ${libxml_VALID})
set01 (WITH_HTML ${libxml_HTML})
set01 (WITH_LEGACY ${libxml_LEGACY})
set01 (WITH_C14N ${libxml_C14N})
set01 (WITH_CATALOG ${libxml_CATALOG})
set01 (WITH_XPATH ${libxml_XPATH})
set01 (WITH_XPTR ${libxml_XPTR})
set01 (WITH_XPTR_LOCS ${libxml_XPTR_LOCS})
set01 (WITH_XINCLUDE ${libxml_XINCLUDE})
set01 (WITH_ICONV ${libxml_ICONV})
set01 (WITH_ICU ${libxml_ICU})
set01 (WITH_ISO8859X ${libxml_ISO8859X})
set01 (WITH_DEBUG ${libxml_DEBUG})
set01 (WITH_REGEXPS ${libxml_REGEXPS})
set01 (WITH_SCHEMAS ${libxml_SCHEMAS})
set01 (WITH_SCHEMATRON ${libxml_SCHEMATRON})
set01 (WITH_MODULES ${libxml_MODULES})
set01 (WITH_ZLIB ${libxml_ZLIB})
set01 (WITH_LZMA ${libxml_LZMA})
configure_file (
    "${libxml_DIR}/include/libxml/xmlversion.h.in"
    "${PROJECT_BINARY_DIR}/include/libxml/xmlversion.h")

set (libxml_C
    "${libxml_DIR}/buf.c"
    "${libxml_DIR}/chvalid.c"
    "${libxml_DIR}/dict.c"
    "${libxml_DIR}/entities.c"
    "${libxml_DIR}/encoding.c"
    "${libxml_DIR}/error.c"
    "${libxml_DIR}/globals.c"
    "${libxml_DIR}/hash.c"
    "${libxml_DIR}/list.c"
    "${libxml_DIR}/parser.c"
    "${libxml_DIR}/parserInternals.c"
    "${libxml_DIR}/SAX2.c"
    "${libxml_DIR}/threads.c"
    "${libxml_DIR}/tree.c"
    "${libxml_DIR}/uri.c"
    "${libxml_DIR}/valid.c"
    "${libxml_DIR}/xmlIO.c"
    "${libxml_DIR}/xmlmemory.c"
    "${libxml_DIR}/xmlstring.c")

add_library (LibXml2 ${libxml_LIB_TYPE}
    ${libxml_C}
    $<$<BOOL:${libxml_C14N}>:${libxml_DIR}/c14n.c>
    $<$<BOOL:${libxml_CATALOG}>:${libxml_DIR}/catalog.c>
    $<$<BOOL:${libxml_DEBUG}>:${libxml_DIR}/debugXML.c>
    $<$<BOOL:${libxml_FTP}>:${libxml_DIR}/nanoftp.c>
    $<$<BOOL:${libxml_HTML}>:${libxml_DIR}/HTMLparser.c ${libxml_DIR}/HTMLtree.c>
    $<$<BOOL:${libxml_HTTP}>:${libxml_DIR}/nanohttp.c>
    $<$<BOOL:${libxml_LEGACY}>:${libxml_DIR}/legacy.c>
    $<$<BOOL:${libxml_LZMA}>:${libxml_DIR}/xzlib.c>
    $<$<BOOL:${libxml_LZMA}>:${libxml_DIR}/xmlmodule.c>
    $<$<BOOL:${libxml_OUTPUT}>:${libxml_DIR}/xmlsave.c>
    $<$<BOOL:${libxml_PATTERN}>:${libxml_DIR}/pattern.c>
    $<$<BOOL:${libxml_READER}>:${libxml_DIR}/xmlreader.c>
    $<$<BOOL:${libxml_REGEXPS}>:${libxml_DIR}/xmlregexp.c ${libxml_DIR}/xmlunicode.c>
    $<$<BOOL:${libxml_SAX1}>:${libxml_DIR}/SAX.c>
    $<$<BOOL:${libxml_SCHEMAS}>:${libxml_DIR}/relaxng.c ${libxml_DIR}/xmlschemas.c ${libxml_DIR}/xmlschemastypes.c>
    $<$<BOOL:${libxml_SCHEMATRON}>:${libxml_DIR}/schematron.c>
    $<$<BOOL:${libxml_WRITER}>:${libxml_DIR}/xmlwriter.c>
    $<$<BOOL:${libxml_XINCLUDE}>:${libxml_DIR}/xinclude.c>
    $<$<BOOL:${libxml_XPATH}>:${libxml_DIR}/xpath.c>
    $<$<BOOL:${libxml_XPTR}>:${libxml_DIR}/xlink.c ${libxml_DIR}/xpointer.c>)
if (libxml_SCHEMAS AND NOT libxml_XPATH)
    target_sources (LibXml2 PRIVATE ${libxml_DIR}/xpath.c)
endif ()

target_include_directories (LibXml2
    PUBLIC
        $<BUILD_INTERFACE:${libxml_DIR}>
        $<BUILD_INTERFACE:${libxml_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

if (libxml_LIB_TYPE STREQUAL "STATIC")
    set_target_properties (LibXml2
        PROPERTIES
            POSITION_INDEPENDENT_CODE ON)
    target_compile_definitions (LibXml2
        PRIVATE
            LIBXML_STATIC)
endif ()

if (libxml_LZMA)
    find_package (LibLZMA REQUIRED)
    target_link_libraries (LibXml2
        PRIVATE
            LibLZMA::LibLZMA)
endif ()

if (libxml_ICU)
    find_package (ICU REQUIRED COMPONENTS data i18n uc)
    target_link_libraries (LibXml2
        PRIVATE
            ICU::data
            ICU::i18n
            ICU::uc)
endif()

if (libxml_THREADS)
    find_package (Threads REQUIRED)
    target_link_libraries (LibXml2
        PRIVATE
            Threads::Threads)
endif ()

add_library (LibXml2::LibXml2 ALIAS LibXml2)
