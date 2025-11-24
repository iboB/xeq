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

    static timer_ptr create(const executor_ptr& s);

    const executor_ptr& get_executor() const {
        return m_executor;
    }
private:
    // sealed interface
    timer(const executor_ptr& strand)
        : m_executor(strand)
    {}
    executor_ptr m_executor;
    friend struct timer_impl;
};

} // namespace xeq
