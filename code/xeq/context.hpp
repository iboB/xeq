// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "executor_ptr.hpp"
#include "work_guard.hpp"
#include <string_view>

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

    [[nodiscard]] work_guard make_work_guard();

    const executor_ptr& get_executor() const noexcept;

    [[nodiscard]] strand_ptr make_strand();

    boost::asio::io_context& as_asio_io_context() noexcept;

    void attach_object(std::string_view name, std::shared_ptr<void> obj);
    [[nodiscard]] std::shared_ptr<void> get_object(std::string_view name) const noexcept;
    std::shared_ptr<void> detach_object(std::string_view name) noexcept;

    struct impl;
private:
    std::unique_ptr<impl> m_impl;
};
} // namespace xeq
