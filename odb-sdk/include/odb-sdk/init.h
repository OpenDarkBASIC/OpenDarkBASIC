#pragma once

#include "odb-sdk/config.h"

ODBSDK_PUBLIC_API int
odbsdk_threadlocal_init(void);

ODBSDK_PUBLIC_API int
odbsdk_init(void);

ODBSDK_PUBLIC_API void
odbsdk_deinit(void);

ODBSDK_PUBLIC_API void
odbsdk_threadlocal_deinit(void);
