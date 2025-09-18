// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "error_code.hpp"
#include <concepts>

namespace xeq {
template <typename T>
concept wait_func_class = std::invocable<T, const error_code&>;
} // namespace xeq
