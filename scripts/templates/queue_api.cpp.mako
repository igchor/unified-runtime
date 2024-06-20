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
 * @file ${name}.cpp
 *
 */

#include "queue_api.hpp"

ur_queue_handle_t_::~ur_queue_handle_t_() {}

%for s in specs:
## FUNCTION ###################################################################
%for obj in th.filter_items(s['objects'], 'type', 'function'):
%if 'ur_queue_handle_t' in th.make_param_lines(n, tags, obj)[0]:
${X}_APIEXPORT ${x}_result_t ${X}_APICALL
${th.make_func_name(n, tags, obj)}(
    %for line in th.make_param_lines(n, tags, obj, format=["name", "type", "delim"]):
    ${line}
    %endfor
    )
{
    <%
        function_name=th.make_func_name(n, tags, obj).replace(x,'')
        function_name=function_name[0].lower() + function_name[1:]
    %> return hQueue->${function_name}( ${", ".join([param for param in th.make_param_lines(n, tags, obj, format=["name"]) if 'hQueue' not in param])} );
}
%endif
%endfor
%endfor
