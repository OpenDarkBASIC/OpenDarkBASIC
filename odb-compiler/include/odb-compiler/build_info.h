#pragma once

#include "odb-compiler/config.h"

ODBCOMPILER_PUBLIC_API const char* build_info_authors(void);
ODBCOMPILER_PUBLIC_API const char* build_info_version(void);
ODBCOMPILER_PUBLIC_API const char* build_info_url(void);
ODBCOMPILER_PUBLIC_API const char* build_info_host(void);
ODBCOMPILER_PUBLIC_API const char* build_info_time(void);
ODBCOMPILER_PUBLIC_API const char* build_info_commit_info(void);
ODBCOMPILER_PUBLIC_API const char* build_info_commit_hash(void);
ODBCOMPILER_PUBLIC_API const char* build_info_compiler(void);
ODBCOMPILER_PUBLIC_API const char* build_info_cmake(void);

ODBCOMPILER_PUBLIC_API const char* build_info_all(void);
