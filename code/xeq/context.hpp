// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "executor.hpp"
#include "work_guard.hpp"

namespace boost::asio {
class io_context;
}

namespace xeq {

class XEQ_API context {
public:
    context();
    explicit context(int concurrency_hint);
    ~context();

    context(const context&) = delete;
    context& operator=(const context&) = delete;

    size_t run();
    size_t poll();

    void stop();
    bool stopped() const;
    void restart();

    work_guard make_work_guard();

    const executor_ptr& get_executor() const noexcept;

    strand_ptr make_strand();

    boost::asio::io_context& as_asio_io_context() noexcept;

    struct impl;
private:
    std::unique_ptr<impl> m_impl;
};
} // namespace xeq
