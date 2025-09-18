// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "coro.hpp"
#include "strand.hpp"
#include "co_spawn.hpp"
#include <stdexcept>

namespace xeq {

inline void co_splice(const strand_ptr& s, coro<void> c) {
    co_spawn(s, std::move(c));
}

inline void co_splice(const executor_ptr& e, coro<void> c) {
    if (!e->is_strand()) {
        throw std::runtime_error("co_splice executor must be a strand");
    }
    co_spawn(e, std::move(c));
}

} // namespace xeq
