include_guard(GLOBAL)

function(plugin_manager_bootloader_setup TARGET_NAME)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    set(oneValueArgs PYTHON_SCRIPTS_PATH GEN_DIR_PATH TOML_DIR_PATH)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "${oneValueArgs}" "")

    message("TODO: Add multiplatform here")
    set(BUILD_PLATFORM "")

    if(WIN32)
        set(BUILD_PLATFORM "win32")
    endif()

    set(DYNAMIC_PLUGIN_ARG "")

    if(BUILD_SHARED_LIBS)
        list(APPEND DYNAMIC_PLUGIN_ARG "--build-dynamic-plugins")
    endif()

    set(GENERATED_INCLUDE_PATH "${arg_GEN_DIR_PATH}/include")
    set(GENERATED_SRC_PATH "${arg_GEN_DIR_PATH}/src")
    set(GENERATED_CMAKE_DIR_PATH "${arg_GEN_DIR_PATH}/cmake")
    set(GENERATED_JSON_DIR_PATH "${arg_GEN_DIR_PATH}/json")

    set(PLUGIN_REGISTRY_TOML_PATH "${arg_TOML_DIR_PATH}/plugin_registry.toml")
    set(REQUESTED_PLUGINS_TOML_PATH "${arg_TOML_DIR_PATH}/requested_plugins.toml")
    set(GENERATED_STATICALLY_RESOLVED_PLUGINS_JSON_PATH "${GENERATED_JSON_DIR_PATH}/statically_resolved_plugins.json")

    # set(SOURCE_PLUGIN_REGISTRY_HEADER_PATH "${CMAKE_CURRENT_LIST_DIR}/plugin_registry.h.in")
    # set(SOURCE_PLUGIN_MANAGER_INTERFACE_DECLARATIONS_PATH "${CMAKE_CURRENT_LIST_DIR}/plugin_manager_interface_declarations.h.in")
    # set(SOURCE_PLUGIN_REGISTRY_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/plugin_registry.c.in")
    # set(SOURCE_PLUGIN_METADATA_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/plugin_metadata.c.in")
    set(SOURCE_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/plugin_manager_bootloader_generated.c.in")

    # set(SOURCE_INIT_CONTEXTS_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/plugin_manager_init_contexts.c.in")
    set(SOURCE_CMAKE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/PluginManagerGenerated.cmake.in")

    # set(GENERATED_PLUGIN_REGISTRY_HEADER_PATH "${GENERATED_INCLUDE_PATH}/plugin_registry.h")
    # set(GENERATED_PLUGIN_MANAGER_INTERFACE_DECLARATIONS_PATH "${GENERATED_INCLUDE_PATH}/plugin_manager_interface_declarations.h")
    # set(GENERATED_PLUGIN_REGISTRY_SRC_PATH "${GENERATED_SRC_PATH}/plugin_registry.c")
    # set(GENERATED_PLUGIN_METADATA_SRC_PATH "${GENERATED_SRC_PATH}/plugin_metadata.c")
    set(GENERATED_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH "${GENERATED_SRC_PATH}/plugin_manager_bootloader_generated.c")

    # set(GENERATED_INIT_CONTEXTS_SRC_PATH "${GENERATED_SRC_PATH}/plugin_manager_init_contexts.c")
    set(GENERATED_CMAKE_PATH "${GENERATED_CMAKE_DIR_PATH}/PluginManagerGenerated.cmake")

    set_property(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${SOURCE_CMAKE_PATH}"
        "${PLUGIN_REGISTRY_TOML_PATH}"
        "${REQUESTED_PLUGINS_TOML_PATH}"
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_generate_cmake.py"
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_generate_templates.py"
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_parse.py"
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_types.py"
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_generate_arguments.py"
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_static_plugin_resolver.py"
    )

    set(PLUGIN_MANAGER_GENERATE_CMAKE_SCRIPT_PATH "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_generate_cmake.py")
    message(STATUS "Running script: ${PLUGIN_MANAGER_GENERATE_CMAKE_SCRIPT_PATH}")
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" "${PLUGIN_MANAGER_GENERATE_CMAKE_SCRIPT_PATH}"
        --target-name "${TARGET_NAME}"
        --build-platform "${BUILD_PLATFORM}"
        ${DYNAMIC_PLUGIN_ARG}

        --plugin-registry-toml-path "${PLUGIN_REGISTRY_TOML_PATH}"
        --requested-plugins-toml-path "${REQUESTED_PLUGINS_TOML_PATH}"

        --source-cmake-path "${SOURCE_CMAKE_PATH}"

        --generated-include-dir-path "${GENERATED_INCLUDE_PATH}"
        # --generated-plugin-registry-src-path "${GENERATED_PLUGIN_REGISTRY_SRC_PATH}"
        # --generated-plugin-metadata-src-path "${GENERATED_PLUGIN_METADATA_SRC_PATH}"
        --generated-plugin-manager-bootloader-generated-src-path "${GENERATED_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH}"
        --generated-cmake-path "${GENERATED_CMAKE_PATH}"
        --generated-statically-resolved-plugins-json-path "${GENERATED_STATICALLY_RESOLVED_PLUGINS_JSON_PATH}"

        COMMAND_ERROR_IS_FATAL ANY
    )
    include("${GENERATED_CMAKE_PATH}")

    message("TODO: Add dependencies on toml files so they recompile when changed for each plugin")

    set(PLUGIN_MANAGER_GENERATE_C_CODE_SCRIPT_PATH "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_generate_c_code.py")
    message(STATUS "Adding custom command script: ${PLUGIN_MANAGER_GENERATE_C_CODE_SCRIPT_PATH}")
    message("TODO: Figure out if toml paths are necessary for this script")
    add_custom_command(
        OUTPUT
        # "${GENERATED_PLUGIN_REGISTRY_HEADER_PATH}"
        # "${GENERATED_PLUGIN_MANAGER_INTERFACE_DECLARATIONS_PATH}"
        # "${GENERATED_PLUGIN_REGISTRY_SRC_PATH}"
        # "${GENERATED_INIT_CONTEXTS_SRC_PATH}"
        ${GENERATED_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH}

        COMMAND "${Python3_EXECUTABLE}" "${PLUGIN_MANAGER_GENERATE_C_CODE_SCRIPT_PATH}"
        --target-name "${TARGET_NAME}"
        --build-platform "${BUILD_PLATFORM}"
        ${DYNAMIC_PLUGIN_ARG}

        --plugin-registry-toml-path "${PLUGIN_REGISTRY_TOML_PATH}"
        --requested-plugins-toml-path "${REQUESTED_PLUGINS_TOML_PATH}"
        --statically-resolved-plugins-json-path "${GENERATED_STATICALLY_RESOLVED_PLUGINS_JSON_PATH}"

        # --source-plugin-registry-header-path "${SOURCE_PLUGIN_REGISTRY_HEADER_PATH}"
        # --source-plugin-manager-interface-declarations-path "${SOURCE_PLUGIN_MANAGER_INTERFACE_DECLARATIONS_PATH}"
        # --source-plugin-registry-src-path "${SOURCE_PLUGIN_REGISTRY_SRC_PATH}"
        # --source-plugin-metadata-src-path "${SOURCE_PLUGIN_METADATA_SRC_PATH}"
        --source-plugin-manager-bootloader-generated-src-path "${SOURCE_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH}"

        # --source-init-contexts-src-path "${SOURCE_INIT_CONTEXTS_SRC_PATH}"

        # --generated-plugin-registry-header-path "${GENERATED_PLUGIN_REGISTRY_HEADER_PATH}"
        # --generated-plugin-manager-interface-declarations-path "${GENERATED_PLUGIN_MANAGER_INTERFACE_DECLARATIONS_PATH}"
        # --generated-plugin-registry-src-path "${GENERATED_PLUGIN_REGISTRY_SRC_PATH}"
        # --generated-plugin-metadata-src-path "${GENERATED_PLUGIN_METADATA_SRC_PATH}"
        --generated-plugin-manager-bootloader-generated-src-path "${GENERATED_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH}"

        # --generated-init-contexts-src-path "${GENERATED_INIT_CONTEXTS_SRC_PATH}"
        DEPENDS
        "${PLUGIN_MANAGER_GENERATE_C_CODE_SCRIPT_PATH}"
        "${PLUGIN_REGISTRY_TOML_PATH}"
        "${REQUESTED_PLUGINS_TOML_PATH}"
        "${GENERATED_STATICALLY_RESOLVED_PLUGINS_JSON_PATH}"

        # "${SOURCE_PLUGIN_REGISTRY_HEADER_PATH}"
        # "${SOURCE_PLUGIN_MANAGER_INTERFACE_DECLARATIONS_PATH}"
        # "${SOURCE_PLUGIN_REGISTRY_SRC_PATH}"
        # "${SOURCE_PLUGIN_METADATA_SRC_PATH}"
        "${SOURCE_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH}"

        # "${SOURCE_INIT_CONTEXTS_SRC_PATH}"
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_generate_templates.py"
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_generate_init_contexts.py"
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_parse.py"
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_types.py"
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_generate_arguments.py"
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_plugin_resolver.py"

        COMMENT "Generating c code files"
        VERBATIM
    )

    set_source_files_properties(

        # "${GENERATED_PLUGIN_REGISTRY_HEADER_PATH}"
        # "${GENERATED_PLUGIN_MANAGER_INTERFACE_DECLARATIONS_PATH}"
        # "${GENERATED_PLUGIN_REGISTRY_SRC_PATH}"
        # "${GENERATED_PLUGIN_METADATA_SRC_PATH}"
        "${GENERATED_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH}"

        # "${GENERATED_INIT_CONTEXTS_SRC_PATH}"
        PROPERTIES GENERATED TRUE
    )
endfunction()
