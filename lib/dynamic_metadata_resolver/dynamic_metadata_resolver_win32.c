#include "dynamic_metadata_resolver.h"

#include <stdint.h>
#include <Windows.h>
#include <assert.h>
#include <stdio.h>

#include <plugin_sdk/plugin_utils.h>

TODO("Allow for support of a logger")
int32_t resolve_get_metadata_fn_dynamic(const char *module_path, const char *target_name, PluginGetMetadata_Fn *out_get_metadata_fn)
{
    assert(module_path != NULL);
    assert(target_name != NULL);
    assert(out_get_metadata_fn != NULL);

    int32_t ret;

    WCHAR wide_path[MAX_PATH];

    ret = (int32_t)MultiByteToWideChar(
        CP_UTF8, // Assume the input is UTF-8 (use CP_ACP for ANSI)
        0, module_path,
        -1, // -1 means the string is null-terminated
        wide_path, MAX_PATH);

    if (ret == 0)
    {
        // DWORD err = GetLastError();
        return ret;
    }

    HMODULE handle = LoadLibrary(wide_path);
    if (!handle)
    {
        TODO("Add error log");
        return -1;
    }

    char function_name[PLUGIN_REGISTRY_MAX_PLUGIN_INTERFACE_NAME_LEN];
    snprintf(function_name, sizeof(function_name), "%s_get_plugin_metadata", target_name);

    FARPROC proc_address = GetProcAddress(handle, function_name);

    if (!proc_address)
    {
        TODO("Add error log");
        FreeLibrary(handle);
        return -2;
    }

    *out_get_metadata_fn = (PluginGetMetadata_Fn)proc_address;

    return 0;
}