// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "timer.hpp"
#include "wait_func_concept.hpp"
#include "executor.hpp"
#include "coro_wobj.hpp"
#include <cassert>

namespace xeq {

class timer_wobj {
    timer_ptr m_timer;
public:
    explicit timer_wobj(const executor_ptr& ex)
        : m_timer(timer::create(ex))
    {
        // the timer will be "hit" from potentially multiple threads
        // if the executor is not a strand itself,
        // this will cause races when notify_one and timer expiry happen at roughly the same time
        // a timer_wobj must be used with a strand
        assert(ex->is_strand());
    }

    const executor_ptr& get_executor() noexcept {
        return m_timer->get_executor();
    }

    void notify_all() {
        get_executor()->post([this] {
            m_timer->cancel();
        });
    }

    void notify_one() {
        get_executor()->post([this] {
            m_timer->cancel_one();
        });
    }

    template <wait_func_class WF>
    void wait(WF&& cb) {
        assert(get_executor()->running_in_this_thread());
        m_timer->expire_never();
        m_timer->add_wait_cb(std::forward<WF>(cb));
    }

    template <wait_func_class WF>
    void wait(timeout to, WF&& cb) {
        assert(get_executor()->running_in_this_thread());
        m_timer->set_timeout(to);
        m_timer->add_wait_cb(std::forward<WF>(cb));
    }

    // corouitne interface implemented in coro_wobj.hpp
    [[nodiscard]] wait_awaitable<timer_wobj> wait() {
        return wait_awaitable(*this);
    }
    [[nodiscard]] timeout_awaitable<timer_wobj> wait(timeout to) {
        return timeout_awaitable(*this, to);
    }
};

} // namespace xeq
