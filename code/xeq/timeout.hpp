// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <chrono>

namespace xeq {

// helper type to represent a timeout

template <typename Rep>
struct t_timeout {
    static_assert(std::is_signed_v<Rep>, "timeout representation must be signed");

    using rep_t = Rep;
    using duration_t = std::chrono::duration<rep_t, std::milli>;
    duration_t duration;

    constexpr t_timeout() = default;

    template <typename Rep2, typename Period>
    constexpr t_timeout(std::chrono::duration<Rep2, Period> d) : duration(std::chrono::ceil<duration_t>(d)) {} // intentionally not explicit

    template <typename Rep2, typename Period>
    static constexpr t_timeout after(std::chrono::duration<Rep2, Period> d) { return t_timeout{d}; }

    static constexpr t_timeout after_ms(rep_t ms) { return after(std::chrono::milliseconds(ms)); }
    static constexpr t_timeout never() { return t_timeout{std::chrono::milliseconds(-1)}; }
    static constexpr t_timeout now() { return t_timeout{std::chrono::milliseconds(0)}; }
    static constexpr t_timeout immediately() { return now(); }

    constexpr rep_t ms() const { return duration.count(); };
    constexpr bool is_infinite() const { return ms() < 0; }
    constexpr bool is_finite() const { return ms() >= 0; }
    constexpr bool is_zero() const { return ms() == 0; }

    constexpr bool operator==(const t_timeout&) const = default;
    constexpr auto operator<=>(const t_timeout& other) const {
        using URep = std::make_unsigned_t<Rep>;
        return URep(ms()) <=> URep(other.ms());
    }
};

// we don't expect huge timeouts, so no point in wasting bits by default
using timeout = t_timeout<int32_t>;

inline namespace timeout_vals {
inline constexpr timeout await_completion_for(std::chrono::milliseconds d) { return timeout::after(d); }
inline constexpr timeout await_completion = timeout::never();
inline constexpr timeout no_wait = timeout::now();
inline constexpr timeout proceed_immediately = timeout::now();
} // namespace timeout_vals

using huge_timeout = t_timeout<int64_t>;

} // namespace xeq
