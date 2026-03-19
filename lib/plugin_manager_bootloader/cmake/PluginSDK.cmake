include_guard(GLOBAL)

function(plugin_generate_register_inc TARGET_NAME)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    set(oneValueArgs TOOLS_DIR_PATH PYTHON_SCRIPTS_PATH GEN_DIR_PATH TOML_DIR_PATH)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "${oneValueArgs}" "")

    set(TOOLS_C_IN_DIR_PATH "${arg_TOOLS_DIR_PATH}/c_in")
    set(GENERATED_INCLUDE_PATH "${arg_GEN_DIR_PATH}/include")

    set(DYNAMIC_PLUGIN_ARG "")

    if(BUILD_SHARED_LIBS)
        list(APPEND DYNAMIC_PLUGIN_ARG "--build-dynamic")
    endif()

    set(MANIFEST_TOML_PATH "${arg_TOML_DIR_PATH}/manifest.toml")
    set(SOURCE_PLUGIN_REGISTER_INC_PATH "${TOOLS_C_IN_DIR_PATH}/plugin_register.c.inc.in")
    set(SOURCE_PLUGIN_DEPENDENCIES_PATH "${TOOLS_C_IN_DIR_PATH}/plugin_dependencies.h.in")
    set(GENERATED_PLUGIN_REGISTER_INC_PATH "${GENERATED_INCLUDE_PATH}/plugin_register.c.inc")
    set(GENERATED_PLUGIN_DEPENDENCIES_PATH "${GENERATED_INCLUDE_PATH}/plugin_dependencies.h")

    set(PLUGIN_SDK_GENERATE_REGISTER_INC_SCRIPT_PATH "${arg_PYTHON_SCRIPTS_PATH}/plugin_sdk_generate_register_inc.py")
    message(STATUS "Adding custom command script: ${PLUGIN_SDK_GENERATE_REGISTER_INC_SCRIPT_PATH}")
    add_custom_command(
        OUTPUT
        "${GENERATED_PLUGIN_REGISTER_INC_PATH}"
        "${GENERATED_PLUGIN_DEPENDENCIES_PATH}"

        COMMAND "${Python3_EXECUTABLE}" "${PLUGIN_SDK_GENERATE_REGISTER_INC_SCRIPT_PATH}"
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
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_generate_templates.py"
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_parse.py"
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_types.py"
        "${arg_PYTHON_SCRIPTS_PATH}/plugin_manager_generate_arguments.py"

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