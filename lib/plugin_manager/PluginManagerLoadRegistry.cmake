include_guard(GLOBAL)

message("TODO: Create new code for statically loading the plugins with dependencies - insert this code under an #if defined(BUILD_STATIC) or something")

function(plugin_manager_load_registry TARGET_NAME JSON_DIR GEN_DIR)
    set(PLUGIN_REGISTRY_JSON_PATH "${JSON_DIR}/plugin_registry.json")

    if(NOT EXISTS "${PLUGIN_REGISTRY_JSON_PATH}")
        message(FATAL_ERROR "Plugin registry at location '${PLUGIN_REGISTRY_JSON_PATH}' not found")
    endif()


    file(READ "${PLUGIN_REGISTRY_JSON_PATH}" PLUGIN_REGISTRY_JSON)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS 
        "${PLUGIN_REGISTRY_JSON_PATH}"
    )

    set(INTERFACE_DEFINITIONS_TEXT "")
    string(JSON INTERFACE_DEFINITIONS_LEN ERROR_VARIABLE JSON_ERR LENGTH "${PLUGIN_REGISTRY_JSON}" interfaces)
    if(JSON_ERR)
        message(FATAL_ERROR "Failed to parse interfaces in plugin registry: ${JSON_ERR}")
    elseif(INTERFACE_DEFINITIONS_LEN EQUAL 0)
        message(FATAL_ERROR "No interfaces found in registry")
    endif() 

    math(EXPR MAX_I "${INTERFACE_DEFINITIONS_LEN} - 1")

    set(PLUGIN_DEFINITIONS_MAX_LEN 0)
    foreach(i RANGE ${MAX_I})
        string(JSON INTERFACE_DEFINITION_JSON GET "${PLUGIN_REGISTRY_JSON}" interfaces ${i})
        string(JSON INTERFACE_NAME GET "${INTERFACE_DEFINITION_JSON}" interface_name)

        string(JSON DEFAULT_PLUGIN_OBJ ERROR_VARIABLE JSON_ERR GET "${INTERFACE_DEFINITION_JSON}" default_plugin)
        if(JSON_ERR)
            message(FATAL_ERROR "Failed to parse default plugin for interface '${INTERFACE_NAME}' in registry: ${JSON_ERR}")
        endif()
        string(JSON DEFAULT_PLUGIN_TYPE TYPE "${INTERFACE_DEFINITION_JSON}" default_plugin)
        set(DEFAULT_PLUGIN_NAME "")

        if(DEFAULT_PLUGIN_TYPE STREQUAL "STRING")
            set(DEFAULT_PLUGIN_NAME "${DEFAULT_PLUGIN_OBJ}")
        elseif(DEFAULT_PLUGIN_TYPE STREQUAL "ARRAY")
            string(JSON DEFAULT_PLUGIN_LIST_LEN LENGTH "${INTERFACE_DEFINITION_JSON}" default_plugin)
            if(DEFAULT_PLUGIN_LIST_LEN EQUAL 0)
                message(FATAL_ERROR "No default plugin found for interface '${INTERFACE_NAME}' in registry")
            endif() 

            MATH(EXPR MAX_J "${DEFAULT_PLUGIN_LIST_LEN} - 1")
            foreach(j RANGE ${MAX_J})
                string(JSON DEFAULT_PLUGIN_JSON GET "${INTERFACE_DEFINITION_JSON}" default_plugin ${j})
                string(JSON DEFAULT_PLUGIN_TARGET GET "${DEFAULT_PLUGIN_JSON}" "target")

                if(DEFAULT_PLUGIN_TARGET STREQUAL "win32" AND WIN32)
                    string(JSON DEFAULT_PLUGIN_NAME GET "${DEFAULT_PLUGIN_JSON}" plugin_name)
                    break()
                endif()
            endforeach()
        endif()

        if(DEFAULT_PLUGIN_NAME STREQUAL "")
            message(FATAL_ERROR "default plugin is not defined for '${INTERFACE_NAME}' in registry")
        endif()

        set(PLUGIN_DEFINITIONS_LIST "")

        string(JSON PLUGINS_LEN ERROR_VARIABLE JSON_ERR LENGTH "${INTERFACE_DEFINITION_JSON}" plugins)
        if(JSON_ERR)
            message(FATAL_ERROR "Failed to parse plugins for interface '${INTERFACE_NAME}' in registry: ${JSON_ERR}")
        elseif(PLUGINS_LEN EQUAL 0)
            message(FATAL_ERROR "No plugins found for interface '${INTERFACE_NAME}' in registry")
        endif() 

        if(PLUGINS_LEN GREATER PLUGIN_DEFINITIONS_MAX_LEN)
            set(PLUGIN_DEFINITIONS_MAX_LEN "${PLUGINS_LEN}")
        endif()

        math(EXPR MAX_J "${PLUGINS_LEN} - 1")
        foreach(j RANGE ${MAX_J})
            string(JSON PLUGIN_JSON GET "${INTERFACE_DEFINITION_JSON}" plugins ${j}) 
            string(JSON PLUGIN_NAME GET "${PLUGIN_JSON}" plugin_name)

            string(JSON PLUGIN_MODULE_PATH GET "${PLUGIN_JSON}" "module_path")
            string(REPLACE "\\" "/" PLUGIN_MODULE_PATH "${PLUGIN_MODULE_PATH}")

            string(APPEND PLUGIN_DEFINITIONS_LIST "
                    {
                        .plugin_name = \"${PLUGIN_NAME}\",
                        .module_path = \"${PLUGIN_MODULE_PATH}\",
                    },
                ")
        endforeach()
        
        string(APPEND INTERFACE_DEFINITIONS_TEXT "
            {
                .interface_name = \"${INTERFACE_NAME}\",
                .default_plugin = \"${DEFAULT_PLUGIN_NAME}\",
                .plugin_definitions = {${PLUGIN_DEFINITIONS_LIST}},
                .plugin_definitions_len = ${PLUGINS_LEN},
            },
        ")

    endforeach()


    set(GENERATED_PLUGIN_REGISTRY_FILE "${CMAKE_CURRENT_BINARY_DIR}/plugin_registry.c")
    configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/plugin_registry.c.in"
        "${GENERATED_PLUGIN_REGISTRY_FILE}"
        @ONLY
        )
    message(STATUS "Configured plugin registry, placed at: ${GENERATED_PLUGIN_REGISTRY_FILE}")

    target_sources(${TARGET_NAME}
        PRIVATE
            "${GENERATED_PLUGIN_REGISTRY_FILE}"
    )

    set(HEADER_CONTENT 
"#pragma once

#include <stddef.h>

typedef struct PluginDefinition
{
    const char *plugin_name;
    const char *module_path;
} PluginDefinition;

typedef struct InterfaceDefinition
{
    const char *interface_name;
    const char *default_plugin;
    PluginDefinition plugin_definitions[${PLUGIN_DEFINITIONS_MAX_LEN}];
    size_t plugin_definitions_len;
} InterfaceDefinition;

typedef struct PluginRegistry
{
    InterfaceDefinition interface_definitions[${INTERFACE_DEFINITIONS_LEN}];
    size_t interface_definitions_len;
} PluginRegistry;

const PluginRegistry *plugin_registry_get(void);
")

    set(HEADER_LOCATION "${GEN_DIR}/plugin_registry.h")
    file(CONFIGURE
        OUTPUT "${HEADER_LOCATION}"
        CONTENT "${HEADER_CONTENT}"
    )

    message(STATUS "Generated plugin manager header at: ${HEADER_LOCATION}")
endfunction()