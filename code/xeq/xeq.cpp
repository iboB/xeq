// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "executor.hpp"
#include "context.hpp"
#include "work_guard.hpp"
#include "timer.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/steady_timer.hpp>

#include <itlib/shared_from.hpp>
#include <itlib/make_ptr.hpp>

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

struct work_guard_impl {
    std::variant<
        asio::executor_work_guard<asio::io_context::executor_type>,
        asio::executor_work_guard<asio::any_io_executor>
    > wg;
};

work_guard executor::make_work_guard() {
    return itlib::make_shared(work_guard_impl{
        boost::asio::make_work_guard(as_asio_executor())
    });
}

work_guard context::make_work_guard() {
    return itlib::make_shared(work_guard_impl{
        boost::asio::make_work_guard(as_asio_io_context())
    });
}

void context::init_executor() {
    m_executor = std::make_shared<context_executor>(asio::io_context::get_executor());
}

timer::~timer() = default; // export vtable

struct timer_impl final : public timer {
public:
    asio::steady_timer m_timer;
    strand_ptr m_strand;

    explicit timer_impl(strand_ptr strand)
        : timer(strand)
        , m_timer(strand->as_asio_executor())
    {}

    virtual size_t expire_after(duration timeFromNow) override {
        return m_timer.expires_after(timeFromNow);
    }
    virtual size_t expire_at(time_point t) override {
        return m_timer.expires_at(t);
    }
    virtual size_t expire_never() override {
        return m_timer.expires_at(time_point::max());
    }

    virtual size_t cancel() override {
        return m_timer.cancel();
    }
    virtual size_t cancel_one() override {
        return m_timer.cancel_one();
    }

    virtual time_point expiry() const override {
        return m_timer.expiry();
    }

    virtual void add_wait_cb(wait_func cb) override {
        m_timer.async_wait(std::move(cb));
    }
};

timer_ptr timer::create(const executor_ptr& ex) {
    return std::make_unique<timer_impl>(ex->make_strand());
}

} // namespace xeq
