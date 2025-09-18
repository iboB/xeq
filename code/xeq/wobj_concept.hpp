// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "timeout.hpp"
#include "error_code.hpp"
#include <concepts>

namespace xeq {

template <typename T>
concept basic_wait_object_class = requires(T wobj) {
    typename T::executor_type;
    wobj.notify_one();
    { wobj.get_executor() } -> std::convertible_to<typename T::executor_type>;
};

template <typename T>
concept wait_object_class =
    basic_wait_object_class<T>
    && requires(T wobj, void(*cb)(const error_code&)) {
        wobj.wait(cb);
    };

template <typename T>
concept timeout_wait_object_class =
    basic_wait_object_class<T>
    && requires(T wobj, timeout to, void(*cb)(const error_code&)) {
        wobj.wait(to, cb);
    };

} // namespace xeq
