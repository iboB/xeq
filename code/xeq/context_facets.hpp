// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "context_facets_declare.hpp"
#include <trex/facets/facets.hpp>

namespace xeq {

// class instead of a typedef so that it can be easily forward-declared
class context_facets : public trex::facets<
    context_facet_domain,
    trex::default_facet_container,
    trex::lock::thread_safe
> {};

} // namespace xeq
