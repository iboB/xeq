// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "timer.hpp"
#include "wait_func_concept.hpp"
#include "executor.hpp"
#include "strand.hpp"
#include "coro_wobj.hpp"
#include <boost/asio/any_io_executor.hpp>
#include <cassert>

namespace xeq {

class timer_wobj {
    // use a strand here
    // the timer will be "hit" from potentially multiple threads
    // if the executor is not a strand itself,
    // this will cause races when notify_one and timer expiry happen at roughly the same time
    strand_ptr m_strand;

    using timer_type = etimer<boost::asio::any_io_executor>;
    timer_type m_timer;
public:
    explicit timer_wobj(strand_ptr s)
        : m_strand(std::move(s))
        , m_timer(m_strand->as_asio_executor())
    {}

    explicit timer_wobj(const executor_ptr& e)
        : timer_wobj(e->make_strand())
    {}

    const strand_ptr& get_executor() noexcept {
        return m_strand;
    }

    void notify_all() {
        m_strand->post([this] {
            m_timer.cancel();
        });
    }

    void notify_one() {
        m_strand->post([this] {
            m_timer.cancel_one();
        });
    }

    template <wait_func_class WF>
    void wait(WF&& cb) {
        assert(m_strand->running_in_this_thread());
        m_timer.expires_never();
        m_timer.async_wait(std::forward<WF>(cb));
    }

    template <wait_func_class WF>
    void wait(timeout to, WF&& cb) {
        assert(m_strand->running_in_this_thread());
        m_timer.expires(to);
        m_timer.async_wait(std::forward<WF>(cb));
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
