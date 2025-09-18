// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "executor.hpp"
#include "work_guard.hpp"
#include <boost/asio/io_context.hpp>

namespace boost::asio {
class io_context;
}

namespace xeq {

class XEQ_API context : public boost::asio::io_context {
public:
    context() {
        init_executor();
    }
    explicit context(int concurrency_hint)
        : boost::asio::io_context(concurrency_hint)
    {
        init_executor();
    }

    work_guard make_work_guard();

    const executor_ptr& get_executor() const noexcept {
        return m_executor;
    }

    strand_ptr make_strand() {
        return m_executor->make_strand();
    }

    boost::asio::io_context& as_asio_io_context() noexcept {
        return *this;
    }

private:
    void init_executor();
    executor_ptr m_executor;
};
} // namespace xeq
