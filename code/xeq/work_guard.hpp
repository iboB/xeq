// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <memory>

namespace xeq {

class XEQ_API work_guard {
public:
    work_guard();
    ~work_guard();

    work_guard(work_guard&&) noexcept;
    work_guard& operator=(work_guard&&) noexcept;

    explicit operator bool() const noexcept { return !!m_impl; }

    void reset();
private:
    struct impl;
    std::unique_ptr<impl> m_impl;

    work_guard(impl* impl);
    friend class executor;
    friend class context;
};

} // namespace xeq
