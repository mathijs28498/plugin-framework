include_guard(GLOBAL)

function(plugin_generate_register_inc TARGET_NAME)
    find_package(Python3 3.7 REQUIRED COMPONENTS Interpreter)

    get_filename_component(PLUGIN_SDK_BUILD_TOOLS_PATH "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/.." ABSOLUTE)

    set(PLUGIN_SDK_CORE_DIR_PATH "${PLUGIN_SDK_BUILD_TOOLS_PATH}/plugin_sdk_core")
    set(PLUGIN_SDK_SCRIPTS_DIR_PATH "${PLUGIN_SDK_BUILD_TOOLS_PATH}/scripts")
    set(PLUGIN_SDK_TEMPLATES_DIR_PATH "${PLUGIN_SDK_CORE_DIR_PATH}/templates")

    set(oneValueArgs GEN_DIR_PATH TOML_DIR_PATH)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "${oneValueArgs}" "")

    set(GENERATED_INCLUDE_PATH "${arg_GEN_DIR_PATH}/include")

    set(DYNAMIC_PLUGIN_ARG "")

    if(BUILD_SHARED_LIBS)
        list(APPEND DYNAMIC_PLUGIN_ARG "--build-dynamic")
    endif()

    set(MANIFEST_TOML_PATH "${arg_TOML_DIR_PATH}/manifest.toml")
    set(SOURCE_PLUGIN_REGISTER_INC_PATH "${PLUGIN_SDK_TEMPLATES_DIR_PATH}/plugin_register.c.inc.in")
    set(SOURCE_PLUGIN_DEPENDENCIES_PATH "${PLUGIN_SDK_TEMPLATES_DIR_PATH}/plugin_dependencies.h.in")
    set(GENERATED_PLUGIN_REGISTER_INC_PATH "${GENERATED_INCLUDE_PATH}/plugin_register.c.inc")
    set(GENERATED_PLUGIN_DEPENDENCIES_PATH "${GENERATED_INCLUDE_PATH}/plugin_dependencies.h")

    set(PLUGIN_SDK_GENERATE_REGISTER_INC_SCRIPT_PATH "${PLUGIN_SDK_SCRIPTS_DIR_PATH}/plugin_sdk_generate.py")
    message("Adding custom command script: ${PLUGIN_SDK_GENERATE_REGISTER_INC_SCRIPT_PATH}")
    add_custom_command(
        OUTPUT
        "${GENERATED_PLUGIN_REGISTER_INC_PATH}"
        "${GENERATED_PLUGIN_DEPENDENCIES_PATH}"

        COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${PLUGIN_SDK_BUILD_TOOLS_PATH}"
        "${Python3_EXECUTABLE}" "${PLUGIN_SDK_GENERATE_REGISTER_INC_SCRIPT_PATH}"

        ${DYNAMIC_PLUGIN_ARG}
        --manifest-toml-path "${MANIFEST_TOML_PATH}"
        --source-plugin-register-inc-path "${SOURCE_PLUGIN_REGISTER_INC_PATH}"
        --source-plugin-dependencies-path "${SOURCE_PLUGIN_DEPENDENCIES_PATH}"
        --generated-plugin-register-inc-path "${GENERATED_PLUGIN_REGISTER_INC_PATH}"
        --generated-plugin-dependencies-path "${GENERATED_PLUGIN_DEPENDENCIES_PATH}"

        DEPENDS
        "${PLUGIN_SDK_GENERATE_REGISTER_INC_SCRIPT_PATH}"
        "${MANIFEST_TOML_PATH}"
        "${SOURCE_PLUGIN_REGISTER_INC_PATH}"
        "${SOURCE_PLUGIN_DEPENDENCIES_PATH}"
        "${PLUGIN_SDK_CORE_DIR_PATH}/__init__.py"
        "${PLUGIN_SDK_CORE_DIR_PATH}/utils.py"
        "${PLUGIN_SDK_CORE_DIR_PATH}/datatypes.py"
        "${PLUGIN_SDK_CORE_DIR_PATH}/parsers/__init__.py"
        "${PLUGIN_SDK_CORE_DIR_PATH}/parsers/manifest_parse.py"
        "${PLUGIN_SDK_CORE_DIR_PATH}/generators/__init__.py"
        "${PLUGIN_SDK_CORE_DIR_PATH}/generators/generate_sdk_files.py"

        COMMENT "Generating register inc header"
        VERBATIM
    )

    set_source_files_properties(
        "${GENERATED_PLUGIN_REGISTER_INC_PATH}"
        "${GENERATED_PLUGIN_DEPENDENCIES_PATH}"
        PROPERTIES
        GENERATED TRUE
        HEADER_FILE_ONLY TRUE
    )

    target_sources(${TARGET_NAME}
        PRIVATE
        "${GENERATED_PLUGIN_REGISTER_INC_PATH}"
        "${GENERATED_PLUGIN_DEPENDENCIES_PATH}"
    )

    target_include_directories(${TARGET_NAME} PRIVATE "${GENERATED_INCLUDE_PATH}")
endfunction()
