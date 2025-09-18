// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "co_spawn.hpp"
#include "context.hpp"
#include "work_guard.hpp"

namespace xeq {

template <typename T>
T co_execute(coro<T> c) {

    // ideally we would have a simple coroutine lambda here,
    // but a gcc asan bug prevents us from using it (it wrongly assumes ref-captures are post-destroy uses)
    struct execute_helper {
        coro<T> cr;
        typename coro<T>::result_type result = itlib::unexpected();
        work_guard guard;

        execute_helper(coro<T> cr, context& ctx)
            : cr(std::move(cr)), guard(ctx.make_work_guard()) {
        }
        coro<void> run() {
            result = co_await cr.safe_result();
            guard.reset();
        }
    };

    context ctx;
    execute_helper helper(std::move(c), ctx);
    co_spawn(ctx, helper.run());
    ctx.run();

    if (helper.result) {
        return std::move(helper.result).value();
    }
    else {
        std::rethrow_exception(std::move(helper.result).error());
    }
}

} // namespace xeq
