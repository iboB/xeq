// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "executor.hpp"
#include "context.hpp"
#include "work_guard.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/any_io_executor.hpp>

#include <itlib/shared_from.hpp>

#include <variant>

namespace asio = boost::asio;
using asio_strand = asio::strand<asio::io_context::executor_type>;

namespace xeq {

namespace {

class context_executor final : public executor, public itlib::enable_shared_from {
public:
    asio::io_context::executor_type m_aexec;
    context_executor(asio::io_context::executor_type&& e)
        : m_aexec(std::move(e))
    {}

    virtual void post(ufunc<void()> func) override {
        asio::post(m_aexec, std::move(func));
    }

    virtual void post_resume(std::coroutine_handle<> handle) override {
        asio::post(m_aexec, [=]() {
            handle.resume();
        });
    }

    virtual bool is_strand() const noexcept override { return false; }

    virtual executor_ptr get_super_executor() noexcept override {
        return shared_from(this);
    }

    boost::asio::any_io_executor as_asio_executor() noexcept override {
        return m_aexec;
    }

    strand_ptr make_strand() override;

    virtual bool running_in_this_thread() const noexcept override {
        return m_aexec.running_in_this_thread();
    }
};

class strand_executor final : public strand, public itlib::enable_shared_from {
public:
    asio_strand m_astrand;
    strand_executor(asio_strand&& s)
        : m_astrand(std::move(s))
    {
    }
    virtual void post(ufunc<void()> func) override {
        asio::post(m_astrand, std::move(func));
    }
    virtual void post_resume(std::coroutine_handle<> handle) override {
        asio::post(m_astrand, [=]() {
            handle.resume();
        });
    }

    executor_ptr get_super_executor() noexcept override {
        auto& ctx = static_cast<context&>(m_astrand.context());
        return ctx.get_executor();
    }

    boost::asio::any_io_executor as_asio_executor() noexcept override {
        return m_astrand;
    }

    strand_ptr make_strand() override {
        return shared_from(this);
    }

    virtual bool running_in_this_thread() const noexcept override {
        return m_astrand.running_in_this_thread();
    }
};

strand_ptr context_executor::make_strand() {
    return std::make_shared<strand_executor>(asio::make_strand(m_aexec));
}

} // namespace

// just export the vtables
strand::~strand() = default;
executor::~executor() = default;

struct work_guard::impl {
    std::variant<
        asio::executor_work_guard<asio::io_context::executor_type>,
        asio::executor_work_guard<asio::any_io_executor>
    > wg;
};

work_guard::work_guard() = default;
work_guard::~work_guard() = default;
work_guard::work_guard(work_guard&&) noexcept = default;
work_guard& work_guard::operator=(work_guard&&) noexcept = default;
void work_guard::reset() { m_impl.reset(); }
work_guard::work_guard(impl* impl) : m_impl(impl) {}

work_guard executor::make_work_guard() {
    return work_guard(new work_guard::impl{
        boost::asio::make_work_guard(as_asio_executor())
    });
}

work_guard context::make_work_guard() {
    return work_guard(new work_guard::impl{
        boost::asio::make_work_guard(as_asio_io_context())
    });
}

void context::init_executor() {
    m_executor = std::make_shared<context_executor>(asio::io_context::get_executor());
}

} // namespace xeq
