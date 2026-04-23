include_guard(GLOBAL)

function(plugin_manager_bootloader_setup TARGET_NAME)
    find_package(Python3 3.7 REQUIRED COMPONENTS Interpreter)

    get_filename_component(INTERNAL_BUILD_TOOLS_PATH "${CMAKE_CURRENT_LIST_DIR}/build_tools" ABSOLUTE)

    set(oneValueArgs PYTHON_SCRIPTS_PATH GEN_DIR_PATH TOML_DIR_PATH PLUGIN_SDK_ROOT_PATH)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "${oneValueArgs}" "")

    set(PLUGIN_SDK_BUILD_TOOLS_PATH "${arg_PLUGIN_SDK_ROOT_PATH}/build_tools")
    set(PYTHON_PATHS
        "${PLUGIN_SDK_BUILD_TOOLS_PATH}"
        "${INTERNAL_BUILD_TOOLS_PATH}"
    )

    cmake_path(CONVERT "${PYTHON_PATHS}" TO_NATIVE_PATH_LIST PYTHON_PATHS_NATIVE)

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
    set(APP_TOML_PATH "${arg_TOML_DIR_PATH}/app.toml")
    set(GENERATED_STATICALLY_RESOLVED_PLUGIN_MANIFACTS_JSON_PATH "${GENERATED_JSON_DIR_PATH}/statically_resolved_plugin_manifests.json")

    set(SOURCE_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/plugin_manager_bootloader_generated.c.in")

    set(SOURCE_CMAKE_PATH "${CMAKE_CURRENT_LIST_DIR}/build_tools/cmake/PluginManagerGenerated.cmake.in")

    set(GENERATED_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH "${GENERATED_SRC_PATH}/plugin_manager_bootloader_generated.c")

    set(GENERATED_CMAKE_PATH "${GENERATED_CMAKE_DIR_PATH}/PluginManagerGenerated.cmake")

    set(PLUGIN_MANAGER_DEPFILE_PATH "${arg_GEN_DIR_PATH}/plugin_manager_generate.d")

    set(PLUGIN_MANAGER_GENERATE_CONFIGURE_SCRIPT_PATH "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_generate_configure.py")
    set_property(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${SOURCE_CMAKE_PATH}"
        "${PLUGIN_REGISTRY_TOML_PATH}"
        "${APP_TOML_PATH}"

        "${INTERNAL_BUILD_TOOLS_PATH}/internal_core/__init__.py"
        "${INTERNAL_BUILD_TOOLS_PATH}/internal_core/datatypes.py"
        "${INTERNAL_BUILD_TOOLS_PATH}/internal_core/plugin_resolver.py"
        "${INTERNAL_BUILD_TOOLS_PATH}/internal_core/generators/__init__.py"
        "${INTERNAL_BUILD_TOOLS_PATH}/internal_core/generators/generate_configure_files.py"
        "${INTERNAL_BUILD_TOOLS_PATH}/internal_core/parsers/__init__.py"
        "${INTERNAL_BUILD_TOOLS_PATH}/internal_core/parsers/plugin_manager_parse.py"
        "${PLUGIN_SDK_BUILD_TOOLS_PATH}/plugin_sdk_core/__init__.py"
        "${PLUGIN_SDK_BUILD_TOOLS_PATH}/plugin_sdk_core/utils.py"
        "${PLUGIN_SDK_BUILD_TOOLS_PATH}/plugin_sdk_core/datatypes.py"
        "${PLUGIN_SDK_BUILD_TOOLS_PATH}/plugin_sdk_core/parsers/__init__.py"
        "${PLUGIN_SDK_BUILD_TOOLS_PATH}/plugin_sdk_core/parsers/manifest_parse.py"

        "${PLUGIN_MANAGER_GENERATE_CONFIGURE_SCRIPT_PATH}"
    )

    message(STATUS "Running script: ${PLUGIN_MANAGER_GENERATE_CONFIGURE_SCRIPT_PATH}")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${PYTHON_PATHS_NATIVE}"
        "${Python3_EXECUTABLE}" "${PLUGIN_MANAGER_GENERATE_CONFIGURE_SCRIPT_PATH}"

        --target-name "${TARGET_NAME}"
        --build-platform "${BUILD_PLATFORM}"
        ${DYNAMIC_PLUGIN_ARG}

        --plugin-registry-toml-path "${PLUGIN_REGISTRY_TOML_PATH}"
        --app-toml-path "${APP_TOML_PATH}"

        --source-cmake-path "${SOURCE_CMAKE_PATH}"

        --generated-include-dir-path "${GENERATED_INCLUDE_PATH}"

        --generated-plugin-manager-bootloader-generated-src-path "${GENERATED_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH}"
        --generated-cmake-path "${GENERATED_CMAKE_PATH}"
        --generated-statically-resolved-plugin-manifests-json-path "${GENERATED_STATICALLY_RESOLVED_PLUGIN_MANIFACTS_JSON_PATH}"

        COMMAND_ERROR_IS_FATAL ANY
    )
    include("${GENERATED_CMAKE_PATH}")

    set(PLUGIN_MANAGER_GENERATE_COMPILE_SCRIPT_PATH "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_generate_compile.py")
    message(STATUS "Adding custom command script: ${PLUGIN_MANAGER_GENERATE_COMPILE_SCRIPT_PATH}")
    add_custom_command(
        OUTPUT
        ${GENERATED_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH}

        COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${PYTHON_PATHS_NATIVE}"
        "${Python3_EXECUTABLE}" "${PLUGIN_MANAGER_GENERATE_COMPILE_SCRIPT_PATH}"

        --target-name "${TARGET_NAME}"
        --build-platform "${BUILD_PLATFORM}"
        ${DYNAMIC_PLUGIN_ARG}

        --plugin-registry-toml-path "${PLUGIN_REGISTRY_TOML_PATH}"
        --app-toml-path "${APP_TOML_PATH}"
        --statically-resolved-plugin-manifests-json-path "${GENERATED_STATICALLY_RESOLVED_PLUGIN_MANIFACTS_JSON_PATH}"

        --source-plugin-manager-bootloader-generated-src-path "${SOURCE_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH}"

        --generated-plugin-manager-bootloader-generated-src-path "${GENERATED_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH}"

        --depfile-path "${PLUGIN_MANAGER_DEPFILE_PATH}"

        DEPENDS
        "${PLUGIN_MANAGER_GENERATE_COMPILE_SCRIPT_PATH}"
        "${PLUGIN_REGISTRY_TOML_PATH}"
        "${APP_TOML_PATH}"
        "${GENERATED_STATICALLY_RESOLVED_PLUGIN_MANIFACTS_JSON_PATH}"

        "${SOURCE_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH}"

        "${INTERNAL_BUILD_TOOLS_PATH}/internal_core/__init__.py"
        "${INTERNAL_BUILD_TOOLS_PATH}/internal_core/plugin_resolver.py"
        "${INTERNAL_BUILD_TOOLS_PATH}/internal_core/datatypes.py"
        "${INTERNAL_BUILD_TOOLS_PATH}/internal_core/memory_pool_size.py"
        "${INTERNAL_BUILD_TOOLS_PATH}/internal_core/generators/__init__.py"
        "${INTERNAL_BUILD_TOOLS_PATH}/internal_core/generators/generate_compile_files.py"
        "${INTERNAL_BUILD_TOOLS_PATH}/internal_core/parsers/__init__.py"
        "${INTERNAL_BUILD_TOOLS_PATH}/internal_core/parsers/plugin_manager_parse.py"
        "${PLUGIN_SDK_BUILD_TOOLS_PATH}/plugin_sdk_core/__init__.py"
        "${PLUGIN_SDK_BUILD_TOOLS_PATH}/plugin_sdk_core/utils.py"
        "${PLUGIN_SDK_BUILD_TOOLS_PATH}/plugin_sdk_core/datatypes.py"
        "${PLUGIN_SDK_BUILD_TOOLS_PATH}/plugin_sdk_core/parsers/__init__.py"
        "${PLUGIN_SDK_BUILD_TOOLS_PATH}/plugin_sdk_core/parsers/manifest_parse.py"
        DEPFILE "${PLUGIN_MANAGER_DEPFILE_PATH}"

        COMMENT "Generating c code files"
        VERBATIM
    )

    set_source_files_properties(
        "${GENERATED_PLUGIN_MANAGER_BOOTLOADER_GENERATED_SRC_PATH}"
        PROPERTIES GENERATED TRUE
    )
endfunction()
