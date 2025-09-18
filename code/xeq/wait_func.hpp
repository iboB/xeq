// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "ufunc.hpp"
#include "wait_func_invoke.hpp"
#include "error_code.hpp"

namespace xeq {
using wait_func = ufunc<void(const error_code& cancelled)>;
} // namespace xeq
