// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "context.hpp"
#include "executor.hpp"
#include "work_guard.hpp"
#include "timer.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/steady_timer.hpp>

#include <itlib/shared_from.hpp>
#include <itlib/make_ptr.hpp>
#include <itlib/data_mutex.hpp>

#include <variant>

namespace asio = boost::asio;
using asio_strand = asio::strand<asio::io_context::executor_type>;

namespace xeq {

namespace {
struct transparent_string_hash : public std::hash<std::string_view> {
    using hash_type = std::hash<std::string_view>;
    using hash_type::operator();
    using is_transparent = void;
};

template <typename T>
using tsumap = std::unordered_map<
    std::string,
    T,
    transparent_string_hash,
    std::equal_to<>
>;
}

struct context::impl : public asio::io_context {
    impl() {
        init_executor();
    }
    impl(int concurrency_hint)
        : asio::io_context(concurrency_hint)
    {
        init_executor();
    }

    void init_executor();
    executor_ptr m_executor;
    itlib::data_mutex<tsumap<std::shared_ptr<void>>, std::mutex> m_objects;
};

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
        auto& ctx = static_cast<context::impl&>(m_astrand.context());
        return ctx.m_executor;
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

void context::impl::init_executor() {
    m_executor = std::make_shared<context_executor>(get_executor());
}

context::context() : m_impl(std::make_unique<impl>()) {}
context::context(int concurrency_hint) : m_impl(std::make_unique<impl>(concurrency_hint)) {}
context::~context() = default;

size_t context::run() {
    return m_impl->run();
}

size_t context::poll() {
    return m_impl->poll();
}

void context::stop() {
    m_impl->stop();
}

bool context::stopped() const {
    return m_impl->stopped();
}

void context::restart() {
    m_impl->restart();
}

const executor_ptr& context::get_executor() const noexcept {
    return m_impl->m_executor;
}

strand_ptr context::make_strand() {
    return m_impl->m_executor->make_strand();
}

boost::asio::io_context& context::as_asio_io_context() noexcept {
    return *m_impl;
}

void context::attach_object(std::string_view name, std::shared_ptr<void> obj) {
    // throw if already exists
    auto [_, inserted] = m_impl->m_objects.unique_lock()->emplace(std::string(name), std::move(obj));
    if (!inserted) {
        throw std::runtime_error("xeq::context::attach_object: object with name '" + std::string(name) + "' already exists");
    }
}

std::shared_ptr<void> context::get_object(std::string_view name) const noexcept {
    auto objects = m_impl->m_objects.unique_lock();
    auto it = objects->find(name);
    if (it == objects->end()) return {};
    return it->second;
}

std::shared_ptr<void> context::detach_object(std::string_view name) noexcept {
    auto objects = m_impl->m_objects.unique_lock();
    auto f = objects->find(name);
    if (f == objects->end()) return {};

    auto ret = f->second;
    objects->erase(f);
    return ret;
}

timer::~timer() = default; // export vtable

struct timer_impl final : public timer {
public:
    asio::steady_timer m_timer;

    explicit timer_impl(executor_ptr strand)
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
    return std::make_unique<timer_impl>(ex);
}

} // namespace xeq
