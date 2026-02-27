include_guard(GLOBAL)

message("TODO: Create new code for statically loading the plugins with dependencies - insert this code under an #if defined(BUILD_STATIC) or something")

function(plugin_manager_load_registry JSON_DIR GEN_DIR)
    file(READ "${JSON_DIR}/plugin_registry.json" PLUGIN_REGISTRY_JSON)

    set(INTERFACE_DEFINITIONS_TEXT "")
    string(JSON INTERFACE_DEFINITIONS_LEN ERROR_VARIABLE err LENGTH "${PLUGIN_REGISTRY_JSON}" interfaces)
    math(EXPR MAX_I "${INTERFACE_DEFINITIONS_LEN} - 1")

    set(PLUGIN_DEFINITIONS_MAX_LEN 0)
    foreach(i RANGE ${MAX_I})
        string(JSON INTERFACE_DEFINITION_JSON GET ${PLUGIN_REGISTRY_JSON} interfaces ${i})
        string(JSON INTERFACE_NAME GET ${INTERFACE_DEFINITION_JSON} interface_name)

        string(JSON DEFAULT_PLUGIN_OBJ GET ${INTERFACE_DEFINITION_JSON} default_plugin)
        string(JSON DEFAULT_PLUGIN_TYPE TYPE ${INTERFACE_DEFINITION_JSON} default_plugin)

        if(DEFAULT_PLUGIN_TYPE STREQUAL "STRING")
            set(DEFAULT_PLUGIN_NAME "${DEFAULT_PLUGIN_OBJ}")
        elseif(DEFAULT_PLUGIN_TYPE STREQUAL "ARRAY")
            string(JSON DEFAULT_PLUGIN_LIST_LEN LENGTH "${INTERFACE_DEFINITION_JSON}" default_plugin)
            MATH(EXPR MAX_J "${DEFAULT_PLUGIN_LIST_LEN} - 1")
            foreach(j RANGE ${MAX_J})
                string(JSON DEFAULT_PLUGIN_JSON GET ${INTERFACE_DEFINITION_JSON} default_plugin ${j})
                string(JSON DEFAULT_PLUGIN_TARGET GET ${DEFAULT_PLUGIN_JSON} "target")

                if(DEFAULT_PLUGIN_TARGET STREQUAL "win32" AND WIN32)
                    string(JSON DEFAULT_PLUGIN_NAME GET ${DEFAULT_PLUGIN_JSON} plugin_name)
                    break()
                endif()
            endforeach()
        endif()

        set(PLUGIN_DEFINITIONS_LIST "")

        string(JSON PLUGINS_LEN ERROR_VARIABLE err LENGTH ${INTERFACE_DEFINITION_JSON} plugins)

        if(PLUGINS_LEN GREATER PLUGIN_DEFINITIONS_MAX_LEN)
            set(PLUGIN_DEFINITIONS_MAX_LEN "${PLUGINS_LEN}")
        endif()

        math(EXPR MAX_J "${PLUGINS_LEN} - 1")
        foreach(j RANGE ${MAX_J})
            string(JSON PLUGIN_JSON GET ${INTERFACE_DEFINITION_JSON} plugins ${j}) 
            string(JSON PLUGIN_NAME GET ${PLUGIN_JSON} plugin_name)
            string(JSON PLUGIN_MODULE_PATH GET ${PLUGIN_JSON} "module_path")

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

    target_sources(plugin_manager
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

    set(HEADER_LOCATION "${GEN_DIR}/__plugin_registry.h")
    file(CONFIGURE
        OUTPUT "${HEADER_LOCATION}"
        CONTENT "${HEADER_CONTENT}"
    )

    message(STATUS "Generated plugin manager header at: ${HEADER_LOCATION}")
endfunction()

function(plugin_manager_load_internal_plugins PLUGIN_REGISTRY_JSON OUT_PLUGIN_MANAGER_HEADER_CONTENT OUT_INTERNAL_PLUGINS_TEXT OUT_INTERNAL_PLUGINS_LEN)
    string(JSON INTERNAL_PLUGINS_LEN ERROR_VARIABLE err LENGTH "${PLUGIN_REGISTRY_JSON}" "internal_plugins")
    math(EXPR MAX_I "${INTERNAL_PLUGINS_LEN} - 1")

    foreach(i RANGE ${MAX_I})
        string(JSON INTERNAL_PLUGIN_JSON GET ${PLUGIN_REGISTRY_JSON} internal_plugins ${i})
        string(JSON INTERFACE_NAME GET "${INTERNAL_PLUGIN_JSON}" interface_name)
        string(JSON PLUGIN_NAME GET "${INTERNAL_PLUGIN_JSON}" plugin_name)
        string(JSON PLUGIN_SOURCE GET "${INTERNAL_PLUGIN_JSON}" plugin_source_path)

        string(JSON INTERNAL_PLUGIN_FW_DECLARATION_COUNT ERROR_VARIABLE err LENGTH ${INTERNAL_PLUGIN_JSON}  plugin_forward_declarations)
        math(EXPR MAX_J "${INTERNAL_PLUGIN_FW_DECLARATION_COUNT} - 1")
        foreach(j RANGE ${MAX_J})
            string(JSON INTERNAL_PLUGIN_FW_DECLARATION GET ${INTERNAL_PLUGIN_JSON} plugin_forward_declarations ${j})
            string(APPEND ${OUT_PLUGIN_MANAGER_HEADER_CONTENT} "${INTERNAL_PLUGIN_FW_DECLARATION}\n")
        endforeach()

        file(READ "${PLUGIN_SOURCE}/${INTERFACE_NAME}_${PLUGIN_NAME}_register.c" PLUGIN_REGISTER_FILE)

        set(INTERFACE_FUNCTION_GET_INTERFACE "NULL")
        set(INTERFACE_FUNCTION_INIT "NULL")
        set(INTERFACE_FUNCTION_SHUTDOWN "NULL")

        string(FIND "${PLUGIN_REGISTER_FILE}" "PLUGIN_REGISTER_INTERFACE" HAS_INTERFACE)
        # TODO: Do an error if file does not have interface registered
        if (NOT HAS_INTERFACE EQUAL -1)
            set(FUNCTION_NAME "${INTERFACE_NAME}_get_interface")
            set(INTERFACE_FUNCTION_GET_INTERFACE "(PluginGetInterface_Fn)${FUNCTION_NAME}")
            string(APPEND ${OUT_PLUGIN_MANAGER_HEADER_CONTENT} "void *${FUNCTION_NAME}(void);\n")
        endif()

        string(FIND "${PLUGIN_REGISTER_FILE}" "PLUGIN_REGISTER_INIT" HAS_INIT)
        if (NOT HAS_INIT EQUAL -1)
            set(FUNCTION_NAME "${INTERFACE_NAME}_init")
            set(INTERFACE_FUNCTION_INIT "(PluginInit_Fn)${FUNCTION_NAME}")
            string(APPEND ${OUT_PLUGIN_MANAGER_HEADER_CONTENT} "int32_t ${FUNCTION_NAME}(void* context);\n")
        endif()

        string(FIND "${PLUGIN_REGISTER_FILE}" "PLUGIN_REGISTER_SHUTDOWN" HAS_SHUTDOWN)
        if (NOT HAS_SHUTDOWN EQUAL -1)
            set(FUNCTION_NAME "${INTERFACE_NAME}_shutdown")
            set(INTERFACE_FUNCTION_INIT "(PluginShutdown_Fn)${FUNCTION_NAME}")
            string(APPEND ${OUT_PLUGIN_MANAGER_HEADER_CONTENT} "int32_t ${FUNCTION_NAME}(void* context);\n")
        endif()

        string(APPEND ${OUT_PLUGIN_MANAGER_HEADER_CONTENT} "\n")

        message("TODO: Handle dependencies")
        string(APPEND ${OUT_INTERNAL_PLUGINS_TEXT} "
            {
                .interface_name = \"${INTERFACE_NAME}\",
                .plugin_name = \"${PLUGIN_NAME}\",
                .dependencies_len = 0,
                .get_interface = ${INTERFACE_FUNCTION_GET_INTERFACE},
                .init = ${INTERFACE_FUNCTION_INIT},
                .shutdown = ${INTERFACE_FUNCTION_SHUTDOWN},
                .is_explicit = false,
                .is_initialized = false,
            },")
    endforeach()

    string(APPEND ${OUT_INTERNAL_PLUGINS_TEXT} "
        ")

    set(${OUT_PLUGIN_MANAGER_HEADER_CONTENT} "${${OUT_PLUGIN_MANAGER_HEADER_CONTENT}}" PARENT_SCOPE)
    set(${OUT_INTERNAL_PLUGINS_TEXT} "${${OUT_INTERNAL_PLUGINS_TEXT}}" PARENT_SCOPE)
    set(${OUT_INTERNAL_PLUGINS_LEN} "${INTERNAL_PLUGINS_LEN}" PARENT_SCOPE)
endfunction()

function(plugin_manager_load_requested_plugins REQUESTED_PLUGINS_JSON OUT_REQUESTED_PLUGINS_TEXT OUT_REQUESTED_PLUGINS_LEN)
    message("TODO: Add recursive dependency resolver for static loading")
    string(JSON REQUESTED_PLUGINS_LEN ERROR_VARIABLE err LENGTH "${REQUESTED_PLUGINS_JSON}" "plugins")
    math(EXPR MAX_I "${REQUESTED_PLUGINS_LEN} - 1")

    foreach(i RANGE ${MAX_I})
        string(JSON REQUESTED_PLUGIN GET ${REQUESTED_PLUGINS_JSON} plugins ${i})
        string(JSON REQUESTED_INTERFACE_NAME GET ${REQUESTED_PLUGIN} interface_name)

        string(JSON REQUESTED_PLUGIN_NAME ERROR_VARIABLE JSON_ERR GET ${REQUESTED_PLUGIN} plugin_name)
        if(JSON_ERR)
            set(REQUESTED_PLUGIN_NAME "")
        endif()

        set(REQUESTED_PLUGIN_IS_EXPLICIT "true")

        string(APPEND ${OUT_REQUESTED_PLUGINS_TEXT} "
            {
                .interface_name = \"${REQUESTED_INTERFACE_NAME}\",
                .plugin_name = \"${REQUESTED_PLUGIN_NAME}\",
                .is_explicit = ${REQUESTED_PLUGIN_IS_EXPLICIT},
                .is_resolved = false,
            },")
    endforeach()

    string(APPEND ${OUT_REQUESTED_PLUGINS_TEXT} "
        ")

    set(${OUT_REQUESTED_PLUGINS_TEXT} "${${OUT_REQUESTED_PLUGINS_TEXT}}" PARENT_SCOPE)
    set(${OUT_REQUESTED_PLUGINS_LEN} "${REQUESTED_PLUGINS_LEN}" PARENT_SCOPE)
endfunction()

function(plugin_manager_load_plugins JSON_DIR GEN_DIR)
    file(READ "${JSON_DIR}/plugin_registry.json" PLUGIN_REGISTRY_JSON)
    file(READ "${JSON_DIR}/plugin_list.json" REQUESTED_PLUGINS_JSON)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS 
        "${JSON_DIR}/plugin_list.json"
        "${JSON_DIR}/plugin_registry.json"
    )

    set(PLUGIN_MANAGER_HEADER_CONTENT "#pragma once\n\n#include <stdint.h>\n\n")
    set(INTERNAL_PLUGINS_TEXT "")
    set(INTERNAL_PLUGINS_LEN)
    plugin_manager_load_internal_plugins(${PLUGIN_REGISTRY_JSON} PLUGIN_MANAGER_HEADER_CONTENT INTERNAL_PLUGINS_TEXT INTERNAL_PLUGINS_LEN)

    set(REQUESTED_PLUGINS_TEXT "")
    set(REQUESTED_PLUGINS_LEN)
    plugin_manager_load_requested_plugins(${REQUESTED_PLUGINS_JSON} REQUESTED_PLUGINS_TEXT REQUESTED_PLUGINS_LEN)

    set(GENERATED_GET_SETUP_CONTEXT_FILE "${CMAKE_CURRENT_BINARY_DIR}/plugin_manager_get_setup_context.c")
    configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/plugin_manager_get_setup_context.c.in"
        "${GENERATED_GET_SETUP_CONTEXT_FILE}"
        @ONLY
        )
    message(STATUS "Configured setup context, placed at: ${GENERATED_GET_SETUP_CONTEXT_FILE}")

    target_sources(plugin_manager
        PRIVATE
            "${GENERATED_GET_SETUP_CONTEXT_FILE}"
    )


    set(HEADER_LOCATION "${GEN_DIR}/__plugin_manager_generated.h")
    file(CONFIGURE
        OUTPUT "${HEADER_LOCATION}"
        CONTENT "${PLUGIN_MANAGER_HEADER_CONTENT}"
    )

    message(STATUS "Generated plugin manager header at: ${HEADER_LOCATION}")
endfunction()