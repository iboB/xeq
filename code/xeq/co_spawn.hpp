// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "context.hpp"

namespace xeq {

inline void co_spawn(const executor_ptr& ex, coro<void> c) {
    auto h = c.take_handle();
    h.promise().m_executor = ex;
    ex->post_resume(h);
}

inline void co_spawn(context& ctx, coro<void> c) {
    co_spawn(ctx.get_executor(), std::move(c));
}

} // namespace xeq
