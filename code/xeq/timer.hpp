// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "timeout.hpp"
#include <boost/asio/steady_timer.hpp>

namespace xeq {
template <typename Executor>
using boost_steady_etimer = boost::asio::basic_waitable_timer<
    boost::asio::steady_timer::clock_type,
    boost::asio::steady_timer::traits_type,
    Executor
>;

template <typename Executor>
class etimer : public boost_steady_etimer<Executor> {
public:
    using super = boost_steady_etimer<Executor>;

    using executor_type = typename super::executor_type;
    using clock_type = typename super::clock_type;

    using super::basic_waitable_timer;

    size_t expires_never() {
        return this->expires_at(clock_type::time_point::max());
    }

    size_t expires(timeout to) {
        if (to.is_infinite()) {
            return this->expires_never();
        }
        return this->expires_after(to.duration);
    }
};

using timer = etimer<boost::asio::any_io_executor>;

} // namespace xeq
