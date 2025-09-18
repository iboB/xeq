// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <splat/symbol_export.h>

#if XEQ_SHARED
#   if BUILDING_XEQ
#       define XEQ_API SYMBOL_EXPORT
#   else
#       define XEQ_API SYMBOL_IMPORT
#   endif
#else
#   define XEQ_API
#endif
