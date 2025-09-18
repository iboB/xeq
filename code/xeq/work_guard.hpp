// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <memory>

namespace xeq {

struct work_guard_impl; // opaque

using work_guard = std::shared_ptr<work_guard_impl>;

} // namespace xeq
