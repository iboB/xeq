// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "executor.hpp"
#include "wait_func.hpp"
#include "wait_func_invoke.hpp"
#include "coro_wobj.hpp"

namespace xeq {

class simple_wobj {
    executor_ptr m_executor;
    wait_func m_cb;
public:
    explicit simple_wobj(const executor_ptr& s) : m_executor(s) {}

    void notify_one() {
        m_executor->post([this] {
            if (m_cb) {
                auto cb = std::exchange(m_cb, nullptr);
                wait_func_invoke_cancelled(cb);
            }
        });
    }

    void wait(wait_func cb) {
        if (m_cb) {
            m_executor->post([old = std::move(m_cb)] {
                wait_func_invoke_cancelled(old);
            });
        }
        m_cb = std::move(cb);
    }

    using executor_type = executor;
    const executor_ptr& get_executor() noexcept {
        return m_executor;
    }

    // corouitne interface implemented in coro_wobj.hpp
    [[nodiscard]] wait_awaitable<simple_wobj> wait() {
        return wait_awaitable(*this);
    }
};

} // namespace xeq
