include_guard(GLOBAL)

function(plugin_manager_setup TARGET_NAME PYTHON_SCRIPTS_PATH JSON_DIR_PATH GEN_DIR_PATH)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    message("TODO: Add multiplatform here")
    set(BUILD_PLATFORM "")
    if(WIN32)
        set(BUILD_PLATFORM "win32")
    endif()

    set(GENERATED_INCLUDE_PATH "${GEN_DIR_PATH}/include")
    set(GENERATED_SRC_PATH "${GEN_DIR_PATH}/src")
    set(GENERATED_CMAKE_DIR_PATH "${GEN_DIR_PATH}/cmake")

    set(PLUGIN_REGISTRY_JSON_PATH "${JSON_DIR_PATH}/plugin_registry.json")
    set(PLUGIN_LIST_JSON_PATH "${JSON_DIR_PATH}/plugin_list.json")

    set(SOURCE_PLUGIN_REGISTRY_HEADER_PATH "${CMAKE_CURRENT_LIST_DIR}/plugin_registry.h.in")
    set(SOURCE_PLUGIN_MANAGER_HEADER_PATH "${CMAKE_CURRENT_LIST_DIR}/plugin_manager_interface_declarations.h.in")
    set(SOURCE_PLUGIN_REGISTRY_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/plugin_registry.c.in")
    set(SOURCE_GET_SETUP_CONTEXT_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/plugin_manager_get_setup_context.c.in")
    set(SOURCE_CMAKE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/PluginManagerGenerated.cmake.in")

    set(GENERATED_PLUGIN_REGISTRY_HEADER_PATH "${GENERATED_INCLUDE_PATH}/plugin_registry.h")
    set(GENERATED_PLUGIN_MANAGER_HEADER_PATH "${GENERATED_INCLUDE_PATH}/plugin_manager_interface_declarations.h")
    set(GENERATED_PLUGIN_REGISTRY_SRC_PATH "${GENERATED_SRC_PATH}/plugin_registry.c")
    set(GENERATED_GET_SETUP_CONTEXT_SRC_PATH "${GENERATED_SRC_PATH}/plugin_manager_get_setup_context.c")
    set(GENERATED_CMAKE_PATH "${GENERATED_CMAKE_DIR_PATH}/PluginManagerGenerated.cmake")

    set_property(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS 
        "${SOURCE_CMAKE_PATH}"
        "${PLUGIN_REGISTRY_JSON_PATH}"
        "${PYTHON_SCRIPTS_PATH}/plugin_manager_generate_cmake.py"
        "${PYTHON_SCRIPTS_PATH}/plugin_manager_generate_templates.py"
        "${PYTHON_SCRIPTS_PATH}/plugin_manager_parse_json.py"
        "${PYTHON_SCRIPTS_PATH}/plugin_manager_types.py"
        "${PYTHON_SCRIPTS_PATH}/plugin_manager_generate_arguments.py"
    )
    
    set(PLUGIN_MANAGER_GENERATE_CMAKE_SCRIPT_PATH "${PYTHON_SCRIPTS_PATH}/plugin_manager_generate_cmake.py")
    message(STATUS "Running script: ${PLUGIN_MANAGER_GENERATE_CMAKE_SCRIPT_PATH}")
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" "${PLUGIN_MANAGER_GENERATE_CMAKE_SCRIPT_PATH}"
            --target-name "${TARGET_NAME}"

            --plugin-registry-json-path "${PLUGIN_REGISTRY_JSON_PATH}"
           
            --source-cmake-path "${SOURCE_CMAKE_PATH}"

            --generated-plugin-registry-src-path "${GENERATED_PLUGIN_REGISTRY_SRC_PATH}"
            --generated-get-setup-context-src-path "${GENERATED_GET_SETUP_CONTEXT_SRC_PATH}"
            --generated-cmake-path "${GENERATED_CMAKE_PATH}"
        COMMAND_ERROR_IS_FATAL ANY
    )

    include("${GENERATED_CMAKE_PATH}")

    set(PLUGIN_MANAGER_GENERATE_C_CODE_SCRIPT_PATH "${PYTHON_SCRIPTS_PATH}/plugin_manager_generate_c_code.py")
    message(STATUS "Running script: ${PLUGIN_MANAGER_GENERATE_C_CODE_SCRIPT_PATH}")
    add_custom_command(
        OUTPUT
            "${GENERATED_PLUGIN_REGISTRY_HEADER_PATH}"
            "${GENERATED_PLUGIN_MANAGER_HEADER_PATH}"
            "${GENERATED_PLUGIN_REGISTRY_SRC_PATH}"
            "${GENERATED_GET_SETUP_CONTEXT_SRC_PATH}"
        COMMAND "${Python3_EXECUTABLE}" "${PLUGIN_MANAGER_GENERATE_C_CODE_SCRIPT_PATH}"
            --target-name "${TARGET_NAME}"
            --build-platform "${BUILD_PLATFORM}"

            --plugin-registry-json-path "${PLUGIN_REGISTRY_JSON_PATH}"
            --plugin-list-json-path "${PLUGIN_LIST_JSON_PATH}"

            --source-plugin-registry-header-path "${SOURCE_PLUGIN_REGISTRY_HEADER_PATH}"
            --source-plugin-manager-header-path "${SOURCE_PLUGIN_MANAGER_HEADER_PATH}"
            --source-plugin-registry-src-path "${SOURCE_PLUGIN_REGISTRY_SRC_PATH}"
            --source-get-setup-context-src-path "${SOURCE_GET_SETUP_CONTEXT_SRC_PATH}"

            --generated-plugin-registry-header-path "${GENERATED_PLUGIN_REGISTRY_HEADER_PATH}"
            --generated-plugin-manager-header-path "${GENERATED_PLUGIN_MANAGER_HEADER_PATH}"
            --generated-plugin-registry-src-path "${GENERATED_PLUGIN_REGISTRY_SRC_PATH}"
            --generated-get-setup-context-src-path "${GENERATED_GET_SETUP_CONTEXT_SRC_PATH}"
        DEPENDS
            "${PLUGIN_MANAGER_GENERATE_C_CODE_SCRIPT_PATH}"
            "${PLUGIN_REGISTRY_JSON_PATH}"
            "${PLUGIN_LIST_JSON_PATH}"
            "${SOURCE_PLUGIN_REGISTRY_HEADER_PATH}"
            "${SOURCE_PLUGIN_MANAGER_HEADER_PATH}"
            "${SOURCE_PLUGIN_REGISTRY_SRC_PATH}"
            "${SOURCE_GET_SETUP_CONTEXT_SRC_PATH}"
            "${PYTHON_SCRIPTS_PATH}/plugin_manager_generate_templates.py"
            "${PYTHON_SCRIPTS_PATH}/plugin_manager_parse_json.py"
            "${PYTHON_SCRIPTS_PATH}/plugin_manager_types.py"
            "${PYTHON_SCRIPTS_PATH}/plugin_manager_generate_arguments.py"
        COMMENT "Generating c code files"
        VERBATIM
    )

    set_source_files_properties(
        "${GENERATED_PLUGIN_REGISTRY_HEADER_PATH}"
        "${GENERATED_PLUGIN_MANAGER_HEADER_PATH}"
        "${GENERATED_PLUGIN_REGISTRY_SRC_PATH}"
        "${GENERATED_GET_SETUP_CONTEXT_SRC_PATH}"
            PROPERTIES GENERATED TRUE
    )

endfunction()

