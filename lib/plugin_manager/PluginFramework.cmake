include_guard(GLOBAL)

function(plugin_framework_add REQUESTED_PLUGINS_ARG INTERFACE_NAME)
    set(ONE_VALUE_ARGS VARIANT)
    cmake_parse_arguments(PFP "" "${ONE_VALUE_ARGS}" "" "${ARGN}")

    if (NOT DEFINED PFP_VARIANT)
        set(PFP_VARIANT "")
    endif()

    list(APPEND ${REQUESTED_PLUGINS_ARG} "${INTERFACE_NAME}|${PFP_VARIANT}|true")

    set(${REQUESTED_PLUGINS_ARG} "${${REQUESTED_PLUGINS_ARG}}" PARENT_SCOPE)
endfunction()

function(plugin_framework_load REQUESTED_PLUGINS_ARG GEN_DIR)
    read_plugin_registry()

    set(X_MACRO_LIST "")
    foreach (REQUESTED_PLUGIN IN LISTS ${REQUESTED_PLUGINS_ARG})
        string(REPLACE "|" ";" REQUESTED_PLUGIN_VALUES "${REQUESTED_PLUGIN}")
        list(GET REQUESTED_PLUGIN_VALUES 0 INTERFACE_NAME)
        list(GET REQUESTED_PLUGIN_VALUES 1 PLUGIN_NAME)
        list(GET REQUESTED_PLUGIN_VALUES 2 IS_EXPLICIT)
        set(PLUGIN_NAME_ARGUMENT "\"${PLUGIN_NAME}\"")
        if (PLUGIN_NAME STREQUAL "")
            set(PLUGIN_NAME_ARGUMENT "NULL")
        endif()
        list(APPEND X_MACRO_LIST "X(\"${INTERFACE_NAME}\", ${PLUGIN_NAME_ARGUMENT}, ${IS_EXPLICIT}) ")
    endforeach()

    set(CONTENT "#pragma once\n\n#define __PLUGIN_FRAMEWORK_PLUGINS_LIST(X) \\\n")

    list(LENGTH X_MACRO_LIST X_MACRO_LINES_LEN)
    foreach(X_MACRO IN LISTS X_MACRO_LIST)
        string(APPEND CONTENT "\t${X_MACRO} \\\n")
    endforeach()
    string(APPEND CONTENT "\t/* end */")

    set(HEADER_LOCATION "${GEN_DIR}/plugin_framework_generated.h")
    file(CONFIGURE
        OUTPUT "${HEADER_LOCATION}"
        CONTENT "${CONTENT}"
    )

    message(STATUS "Generated plugin header at ${HEADER_LOCATION}")
endfunction()

function(read_plugin_registry)
    cmake_path(GET CMAKE_CURRENT_LIST_DIR PARENT_PATH JSON_DIR)
    file(READ "${JSON_DIR}/plugin_registry.json" PLUGIN_REGISTRY_JSON)

    # message("json: ${PLUGIN_REGISTRY_JSON}")
endfunction()