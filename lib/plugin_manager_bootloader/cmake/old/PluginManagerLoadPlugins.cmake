include_guard(GLOBAL)

function(plugin_manager_load_internal_plugins PLUGIN_REGISTRY_JSON OUT_PLUGIN_MANAGER_HEADER_CONTENT OUT_INTERNAL_PLUGINS_TEXT OUT_INTERNAL_PLUGINS_LEN OUT_INTERNAL_PLUGIN_TARGET_LIST)
    set(INTERNAL_PLUGINS_TEXT "")
    set(PLUGIN_MANAGER_HEADER_CONTENT "${${OUT_PLUGIN_MANAGER_HEADER_CONTENT}}")
    set(INTERNAL_PLUGIN_TARGET_LIST "")

    string(JSON INTERNAL_PLUGINS_LEN ERROR_VARIABLE JSON_ERR LENGTH "${PLUGIN_REGISTRY_JSON}" "internal_plugins")
    if(JSON_ERR)
        message(FATAL_ERROR "Failed to parse internal plugins in registry: ${JSON_ERR}")
    elseif(INTERNAL_PLUGINS_LEN EQUAL 0)
        message(FATAL_ERROR "No internal plugins found in registry")
    endif() 

    math(EXPR MAX_I "${INTERNAL_PLUGINS_LEN} - 1")

    foreach(i RANGE ${MAX_I})
        string(JSON INTERNAL_PLUGIN_JSON GET "${PLUGIN_REGISTRY_JSON}" internal_plugins ${i})
        string(JSON INTERFACE_NAME GET "${INTERNAL_PLUGIN_JSON}" interface_name)
        string(JSON PLUGIN_NAME GET "${INTERNAL_PLUGIN_JSON}" plugin_name)
        string(JSON PLUGIN_SOURCE GET "${INTERNAL_PLUGIN_JSON}" plugin_source_path)
        string(JSON PLUGIN_TARGET_NAME GET "${INTERNAL_PLUGIN_JSON}" cmake_target_name)

        if (NOT TARGET ${PLUGIN_TARGET_NAME})
            add_subdirectory(
                "${PLUGIN_SOURCE}" 
                "${CMAKE_CURRENT_BINARY_DIR}/internal_plugins/${PLUGIN_TARGET_NAME}"
            )
        endif()

        list(APPEND INTERNAL_PLUGIN_TARGET_LIST "${PLUGIN_TARGET_NAME}")


        string(JSON INTERNAL_PLUGIN_FW_DECLARATION_COUNT ERROR_VARIABLE JSON_ERR LENGTH ${INTERNAL_PLUGIN_JSON}  plugin_forward_declarations)
        if(NOT (JSON_ERR OR INTERNAL_PLUGIN_FW_DECLARATION_COUNT EQUAL 0))
            math(EXPR MAX_J "${INTERNAL_PLUGIN_FW_DECLARATION_COUNT} - 1")
            foreach(j RANGE ${MAX_J})
                string(JSON INTERNAL_PLUGIN_FW_DECLARATION GET "${INTERNAL_PLUGIN_JSON}" plugin_forward_declarations ${j})
                string(APPEND PLUGIN_MANAGER_HEADER_CONTENT "${INTERNAL_PLUGIN_FW_DECLARATION}\n")
            endforeach()
        endif()


        set(PLUGIN_REGISTER_PATH "${PLUGIN_SOURCE}/${INTERFACE_NAME}_${PLUGIN_NAME}_register.c")
        if(NOT EXISTS "${PLUGIN_REGISTER_PATH}")
            message(FATAL_ERROR "internal plugin '${INTERFACE_NAME}' '${PLUGIN_NAME}' does not have a register file: ${PLUGIN_REGISTER_PATH}")
        endif()

        file(READ "${PLUGIN_REGISTER_PATH}" PLUGIN_REGISTER_FILE)
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS 
            "${PLUGIN_REGISTER_PATH}"
        )

        set(INTERFACE_FUNCTION_GET_INTERFACE "NULL")
        set(INTERFACE_FUNCTION_INIT "NULL")
        set(INTERFACE_FUNCTION_SHUTDOWN "NULL")

        # TODO: Add better way of checking which capabilities it has
        string(REGEX MATCH "\n[ \t]*PLUGIN_REGISTER_INTERFACE" HAS_INTERFACE "${PLUGIN_REGISTER_FILE}")
        if (HAS_INTERFACE STREQUAL "")
            message(FATAL_ERROR "Plugin '${INTERFACE_NAME}' '${PLUGIN_NAME}' does not have an interface registered in file: ${PLUGIN_REGISTER_PATH}")
        endif()
        
        set(FUNCTION_NAME "${INTERFACE_NAME}_get_interface")
        set(INTERFACE_FUNCTION_GET_INTERFACE "(PluginGetInterface_Fn)${FUNCTION_NAME}")
        string(APPEND PLUGIN_MANAGER_HEADER_CONTENT "void *${FUNCTION_NAME}(void);\n")

        string(REGEX MATCH "\n[ \t]*PLUGIN_REGISTER_INIT" HAS_INIT "${PLUGIN_REGISTER_FILE}")
        if (NOT HAS_INIT STREQUAL "")
            set(FUNCTION_NAME "${INTERFACE_NAME}_init")
            set(INTERFACE_FUNCTION_INIT "(PluginInit_Fn)${FUNCTION_NAME}")
            string(APPEND PLUGIN_MANAGER_HEADER_CONTENT "int32_t ${FUNCTION_NAME}(void* context);\n")
        endif()

        string(REGEX MATCH "\n[ \t]*PLUGIN_REGISTER_SHUTDOWN" HAS_SHUTDOWN "${PLUGIN_REGISTER_FILE}")
        if (NOT HAS_SHUTDOWN STREQUAL "")
            set(FUNCTION_NAME "${INTERFACE_NAME}_shutdown")
            set(INTERFACE_FUNCTION_SHUTDOWN "(PluginShutdown_Fn)${FUNCTION_NAME}")
            string(APPEND PLUGIN_MANAGER_HEADER_CONTENT "int32_t ${FUNCTION_NAME}(void* context);\n")
        endif()

        string(APPEND PLUGIN_MANAGER_HEADER_CONTENT "\n")

        # TODO: Handle dependencies
        string(APPEND INTERNAL_PLUGINS_TEXT "
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

    string(APPEND INTERNAL_PLUGINS_TEXT "
        ")

    set(${OUT_PLUGIN_MANAGER_HEADER_CONTENT} "${PLUGIN_MANAGER_HEADER_CONTENT}" PARENT_SCOPE)
    set(${OUT_INTERNAL_PLUGINS_TEXT} "${INTERNAL_PLUGINS_TEXT}" PARENT_SCOPE)
    set(${OUT_INTERNAL_PLUGINS_LEN} "${INTERNAL_PLUGINS_LEN}" PARENT_SCOPE)
    set(${OUT_INTERNAL_PLUGIN_TARGET_LIST} "${INTERNAL_PLUGIN_TARGET_LIST}" PARENT_SCOPE)
endfunction()

function(plugin_manager_load_requested_plugins_dynamic REQUESTED_PLUGINS_JSON OUT_REQUESTED_PLUGINS_TEXT OUT_REQUESTED_PLUGINS_LEN OUT_PLUGIN_PROVIDERS_TEXT OUT_PLUGIN_PROVIDERS_LEN)
    set(REQUESTED_PLUGINS_TEXT "")
    string(JSON REQUESTED_PLUGINS_LEN ERROR_VARIABLE JSON_ERR LENGTH "${REQUESTED_PLUGINS_JSON}" "plugins")
    if(JSON_ERR)
        message(FATAL_ERROR "Failed to parse plugins in plugin_list: ${JSON_ERR}")
    elseif(REQUESTED_PLUGINS_LEN EQUAL 0)
        message(FATAL_ERROR "No plugins found in plugin_list")
    endif() 

    math(EXPR MAX_I "${REQUESTED_PLUGINS_LEN} - 1")

    foreach(i RANGE ${MAX_I})
        string(JSON REQUESTED_PLUGIN GET "${REQUESTED_PLUGINS_JSON}" plugins ${i})
        string(JSON REQUESTED_INTERFACE_NAME GET "${REQUESTED_PLUGIN}" interface_name)

        string(JSON REQUESTED_PLUGIN_NAME ERROR_VARIABLE JSON_ERR GET "${REQUESTED_PLUGIN}" plugin_name)
        if(JSON_ERR)
            set(REQUESTED_PLUGIN_NAME "")
        endif()

        set(REQUESTED_PLUGIN_IS_EXPLICIT "true")

        string(APPEND REQUESTED_PLUGINS_TEXT "
            {
                .interface_name = \"${REQUESTED_INTERFACE_NAME}\",
                .plugin_name = \"${REQUESTED_PLUGIN_NAME}\",
                .is_explicit = ${REQUESTED_PLUGIN_IS_EXPLICIT},
                .is_resolved = false,
            },")
    endforeach()

    string(APPEND REQUESTED_PLUGINS_TEXT "
        ")

    set(${OUT_REQUESTED_PLUGINS_TEXT} "${REQUESTED_PLUGINS_TEXT}" PARENT_SCOPE)
    set(${OUT_REQUESTED_PLUGINS_LEN} "${REQUESTED_PLUGINS_LEN}" PARENT_SCOPE)
    set(${OUT_PLUGIN_PROVIDERS_TEXT} "0" PARENT_SCOPE)
    set(${OUT_PLUGIN_PROVIDERS_LEN} "0" PARENT_SCOPE)
endfunction()

function(plugin_manager_load_requested_plugins_static REQUESTED_PLUGINS_JSON OUT_REQUESTED_PLUGINS_TEXT OUT_REQUESTED_PLUGINS_LEN OUT_PLUGIN_PROVIDERS_TEXT OUT_PLUGIN_PROVIDERS_LEN)
    # TODO: Implement this
    # TODO: Add recursive dependency resolver for static loading
    set(${OUT_REQUESTED_PLUGINS_TEXT} "0" PARENT_SCOPE)
    set(${OUT_REQUESTED_PLUGINS_LEN} "0" PARENT_SCOPE)
    set(${OUT_PLUGIN_PROVIDERS_TEXT} "0" PARENT_SCOPE)
    set(${OUT_PLUGIN_PROVIDERS_LEN} "0" PARENT_SCOPE)
endfunction()

function(plugin_manager_load_requested_plugins REQUESTED_PLUGINS_JSON OUT_REQUESTED_PLUGINS_TEXT OUT_REQUESTED_PLUGINS_LEN OUT_PLUGIN_PROVIDERS_TEXT OUT_PLUGIN_PROVIDERS_LEN)
    set(REQUESTED_PLUGINS_TEXT)
    set(REQUESTED_PLUGINS_LEN)
    set(PLUGIN_PROVIDERS_TEXT)
    set(PLUGIN_PROVIDERS_LEN)
    if(TRUE)
        plugin_manager_load_requested_plugins_dynamic("${REQUESTED_PLUGINS_JSON}" REQUESTED_PLUGINS_TEXT REQUESTED_PLUGINS_LEN PLUGIN_PROVIDERS_TEXT PLUGIN_PROVIDERS_LEN)
    else()
        plugin_manager_load_requested_plugins_static("${REQUESTED_PLUGINS_JSON}" REQUESTED_PLUGINS_TEXT REQUESTED_PLUGINS_LEN PLUGIN_PROVIDERS_TEXT PLUGIN_PROVIDERS_LEN)
    endif()
    set(${OUT_REQUESTED_PLUGINS_TEXT} "${REQUESTED_PLUGINS_TEXT}" PARENT_SCOPE)
    set(${OUT_REQUESTED_PLUGINS_LEN} "${REQUESTED_PLUGINS_LEN}" PARENT_SCOPE)
    set(${OUT_PLUGIN_PROVIDERS_TEXT} "${PLUGIN_PROVIDERS_TEXT}" PARENT_SCOPE)
    set(${OUT_PLUGIN_PROVIDERS_LEN} "${PLUGIN_PROVIDERS_LEN}" PARENT_SCOPE)
endfunction()

function(plugin_manager_load_plugins TARGET_NAME JSON_DIR GEN_DIR)
    set(PLUGIN_REGISTRY_JSON_PATH "${JSON_DIR}/plugin_registry.json")
    set(PLUGIN_LIST_JSON_PATH "${JSON_DIR}/plugin_list.json")

    if(NOT EXISTS "${PLUGIN_REGISTRY_JSON_PATH}")
        message(FATAL_ERROR "Plugin registry at location '${PLUGIN_REGISTRY_JSON_PATH}' not found")
    endif()

    if(NOT EXISTS "${PLUGIN_LIST_JSON_PATH}")
        message(FATAL_ERROR "Plugin list at location '${PLUGIN_LIST_JSON_PATH}' not found")
    endif()

    file(READ "${PLUGIN_REGISTRY_JSON_PATH}" PLUGIN_REGISTRY_JSON)
    file(READ "${PLUGIN_LIST_JSON_PATH}" REQUESTED_PLUGINS_JSON)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS 
        "${PLUGIN_REGISTRY_JSON_PATH}"
        "${PLUGIN_LIST_JSON_PATH}"
    )

    set(PLUGIN_MANAGER_HEADER_CONTENT "#pragma once
    
#include <stdint.h>

")
    set(INTERNAL_PLUGINS_TEXT)
    set(INTERNAL_PLUGINS_LEN)
    set(REQUESTED_PLUGINS_TEXT)
    set(REQUESTED_PLUGINS_LEN)
    set(PLUGIN_PROVIDERS_TEXT)
    set(PLUGIN_PROVIDERS_LEN)
    set(INTERNAL_PLUGIN_TARGET_LIST)

    plugin_manager_load_internal_plugins("${PLUGIN_REGISTRY_JSON}" PLUGIN_MANAGER_HEADER_CONTENT INTERNAL_PLUGINS_TEXT INTERNAL_PLUGINS_LEN INTERNAL_PLUGIN_TARGET_LIST)
    plugin_manager_load_requested_plugins("${REQUESTED_PLUGINS_JSON}" REQUESTED_PLUGINS_TEXT REQUESTED_PLUGINS_LEN PLUGIN_PROVIDERS_TEXT PLUGIN_PROVIDERS_LEN)

    set(GENERATED_GET_SETUP_CONTEXT_FILE "${CMAKE_CURRENT_BINARY_DIR}/plugin_manager_get_setup_context.c")
    configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/plugin_manager_get_setup_context.c.in"
        "${GENERATED_GET_SETUP_CONTEXT_FILE}"
        @ONLY
        )
    message(STATUS "Configured setup context, placed at: ${GENERATED_GET_SETUP_CONTEXT_FILE}")

    target_sources(${TARGET_NAME}
        PRIVATE
            "${GENERATED_GET_SETUP_CONTEXT_FILE}"
    )

    if(NOT "${INTERNAL_PLUGIN_TARGET_LIST}" STREQUAL "")
        message("Linking: ${INTERNAL_PLUGIN_TARGET_LIST}")
        target_link_libraries(${TARGET_NAME}
            PRIVATE
                ${INTERNAL_PLUGIN_TARGET_LIST}
        ) 

    endif()
    


    set(HEADER_LOCATION "${GEN_DIR}/plugin_manager_generated.h")
    file(CONFIGURE
        OUTPUT "${HEADER_LOCATION}"
        CONTENT "${PLUGIN_MANAGER_HEADER_CONTENT}"
    )

    message(STATUS "Generated plugin manager header at: ${HEADER_LOCATION}")
endfunction()