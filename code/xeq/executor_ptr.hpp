// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <memory>

namespace xeq {

class executor;
class strand;

using executor_ptr = std::shared_ptr<executor>;
using strand_ptr = std::shared_ptr<strand>;

} // namespace xeq
