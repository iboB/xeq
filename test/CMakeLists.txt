# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
CPMAddPackage(gh:iboB/doctest-util@0.1.4)
set_target_properties(doctest PROPERTIES FOLDER test)
set_target_properties(doctest-main PROPERTIES FOLDER test)

CPMAddPackage(gh:iboB/b_stacktrace@0.23)

macro(xeq_test test)
    add_doctest_lib_test(${test} xeq t-${test}.cpp ${ARGN})
endmacro()

xeq_test(timeout)
xeq_test(thread_runner)

xeq_test(coro)
xeq_test(coro-mt)
xeq_test(coro-stack LIBRARIES b_stacktrace::b_stacktrace)
xeq_test(generator)
