# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
add_executable(xeq-sandbox
    sandbox.cpp
)
target_link_libraries(xeq-sandbox xeq::xeq)
set_target_properties(xeq-sandbox PROPERTIES FOLDER sandbox)
