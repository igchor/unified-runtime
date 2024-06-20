<%!
import re
from templates import helper as th
%><%
    n=namespace
    N=n.upper()

    x=tags['$x']
    X=x.upper()
%>/*
 *
 * Copyright (C) 2024 Intel Corporation
 *
 * Part of the Unified-Runtime Project, under the Apache License v2.0 with LLVM Exceptions.
 * See LICENSE.TXT
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 * @file ${name}.hpp
 *
 */

#pragma once

#include <ur_api.h>

struct ur_queue_handle_t_ {
    virtual ~ur_queue_handle_t_();
    %for s in specs:
    ## FUNCTION ###################################################################
    %for obj in th.filter_items(s['objects'], 'type', 'function'):
    %if 'ur_queue_handle_t' in th.make_param_lines(n, tags, obj, format=["type"])[0]:
    <%
        function_name=th.make_func_name(n, tags, obj).replace(x,'')
        function_name=function_name[0].lower() + function_name[1:]
    %>virtual ${x}_result_t ${function_name}( ${", ".join([param for param in th.make_param_lines(n, tags, obj, format=["type"]) if 'ur_queue_handle_t' not in param])} ) = 0;
    %endif
    %endfor
    %endfor
};
