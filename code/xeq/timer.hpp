// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "executor_ptr.hpp"
#include "wait_func.hpp"
#include "timeout.hpp"
#include <chrono>
#include <cstddef>
#include <memory>

namespace xeq {

class timer;
using timer_ptr = std::unique_ptr<timer>;

class XEQ_API timer {
public:
    virtual ~timer();

    using clock_type = std::chrono::steady_clock;
    using duration = clock_type::duration;
    using time_point = clock_type::time_point;

    timer(const timer&) = delete;
    timer& operator=(const timer&) = delete;

    virtual size_t expire_after(duration t_from_now) = 0;
    virtual size_t expire_at(time_point t) = 0;
    virtual size_t expire_never() = 0;

    size_t set_timeout(timeout t) {
        if (t.is_infinite()) {
            return expire_never();
        }
        return expire_after(t.duration);
    }

    virtual size_t cancel() = 0;
    virtual size_t cancel_one() = 0;

    virtual time_point expiry() const = 0;

    virtual void add_wait_cb(wait_func cb) = 0;

    // will create a strand from the executor if it's not a strand itself
    // the timer will be "hit" from potentially multiple threads
    // if the executor is not a strand itself,
    // this will cause races when notify_one and timer expiry happen at roughly the same time
    // thus the timer executor is always a strand
    static timer_ptr create(const executor_ptr& s);

    const strand_ptr& get_executor() const {
        return m_strand;
    }
private:
    // sealed interface
    timer(const strand_ptr& strand)
        : m_strand(strand)
    {}
    strand_ptr m_strand;
    friend struct timer_impl;
};

} // namespace xeq
