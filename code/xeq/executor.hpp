// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "ufunc.hpp"
#include "work_guard.hpp"
#include <coroutine>
#include <memory>

namespace boost::asio {
class any_io_executor;
}

namespace xeq {

class executor;
class strand;

using executor_ptr = std::shared_ptr<executor>;
using strand_ptr = std::shared_ptr<strand>;

class XEQ_API executor {
public:
    work_guard make_work_guard();

    virtual void post(ufunc<void()> func) = 0;
    virtual void post_resume(std::coroutine_handle<> handle) = 0;

    virtual bool is_strand() const noexcept = 0;

    virtual executor_ptr get_super_executor() noexcept = 0;

    virtual strand_ptr make_strand() = 0;

    virtual bool running_in_this_thread() const noexcept = 0;

    virtual boost::asio::any_io_executor as_asio_executor() noexcept = 0;

protected:
    // protected as it's only managed by shared_ptr
    // virtual so as to export the vtable
    virtual ~executor();
};

class XEQ_API strand : public executor {
public:
    virtual bool is_strand() const noexcept final override { return true; }

protected:
    // as with executor, we're hiding the implementation
    virtual ~strand();
};

} // namespace xeq
