# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
include(icm_add_lib)
CPMAddPackage(gh:iboB/splat@1.3.4)
CPMAddPackage(gh:iboB/itlib@1.11.7)

if(NOT TARGET Boost::boost)
    CPMAddPackage(
        NAME Boost
        VERSION 1.87.0
        URL https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.tar.xz
        URL_HASH SHA256=7da75f171837577a52bbf217e17f8ea576c7c246e4594d617bfde7fafd408be5
        SYSTEM TRUE
        OPTIONS "BOOST_ENABLE_CMAKE ON"
                "BOOST_INCLUDE_LIBRARIES asio"
    )
endif()

if(NOT TARGET Boost::asio)
    add_library(Boost::asio ALIAS Boost::boost)
endif()

icm_add_lib(xeq XEQ)
add_library(xeq::xeq ALIAS xeq)

target_sources(xeq
    INTERFACE FILE_SET HEADERS FILES
        xeq/api.h
        xeq/coro.hpp
    PRIVATE
        xeq/thread_name.cpp
        xeq/xeq.cpp
)

target_link_libraries(xeq
    PUBLIC
        splat::splat
        itlib::itlib
    PRIVATE
        Boost::asio
)
