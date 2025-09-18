# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
include(icm_add_lib)

icm_add_lib(xeq XEQ)
add_library(xeq::xeq ALIAS xeq)

CPMAddPackage(gh:iboB/splat@1.3.4)

target_link_libraries(xeq PUBLIC
    splat::splat
)

target_sources(xeq PRIVATE
    xeq/api.h
    xeq/xeq.cpp
)

