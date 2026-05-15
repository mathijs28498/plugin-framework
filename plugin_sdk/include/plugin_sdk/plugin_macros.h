#pragma once

#define VTABLE_METHOD_CALL_NO_ARGS(iface, method_name) \
    (iface)->vtable->method_name((iface)->context)

#define VTABLE_METHOD_CALL(iface, method_name, ...) \
    (iface)->vtable->method_name((iface)->context, ##__VA_ARGS__)
