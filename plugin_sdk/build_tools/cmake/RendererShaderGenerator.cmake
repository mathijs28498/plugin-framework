include_guard(GLOBAL)

function(target_embed_shader TARGET_NAME)
    find_package(Python3 3.7 REQUIRED COMPONENTS Interpreter)

    get_filename_component(PLUGIN_SDK_BUILD_TOOLS_PATH "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/.." ABSOLUTE)

    set(PLUGIN_SDK_CORE_DIR_PATH "${PLUGIN_SDK_BUILD_TOOLS_PATH}/plugin_sdk_core")
    set(PLUGIN_SDK_SCRIPTS_DIR_PATH "${PLUGIN_SDK_BUILD_TOOLS_PATH}/scripts")
    set(PLUGIN_SDK_TEMPLATES_DIR_PATH "${PLUGIN_SDK_CORE_DIR_PATH}/templates")

    set(oneValueArgs GEN_DIR_PATH SHADER_FILE_PATH)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "${oneValueArgs}" "")

    get_filename_component(SHADER_NAME "${arg_SHADER_FILE_PATH}" NAME_WLE)

    string(REPLACE "." "_" SHADER_NAME ${SHADER_NAME})

    message("TODO: Add a shader folder")

    set(SOURCE_HEADER_PATH "${PLUGIN_SDK_TEMPLATES_DIR_PATH}/shader.h.in")
    set(SOURCE_SOURCE_PATH "${PLUGIN_SDK_TEMPLATES_DIR_PATH}/shader.c.in")

    set(GENERATED_HEADER_PATH "${arg_GEN_DIR_PATH}/${SHADER_NAME}.h")
    set(GENERATED_SOURCE_PATH "${arg_GEN_DIR_PATH}/${SHADER_NAME}.c")

    set(RENDERER_SHADER_GENERATE_SCRIPT_PATH "${PLUGIN_SDK_SCRIPTS_DIR_PATH}/renderer_shader_generate.py")
    message("Adding custom command script: ${RENDERER_SHADER_GENERATE_SCRIPT_PATH}")

    add_custom_command(
        OUTPUT
        "${GENERATED_HEADER_PATH}"
        "${GENERATED_SOURCE_PATH}"

        COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${PLUGIN_SDK_BUILD_TOOLS_PATH}"
        "${Python3_EXECUTABLE}" "${RENDERER_SHADER_GENERATE_SCRIPT_PATH}"

        --shader-file-path "${arg_SHADER_FILE_PATH}"
        --shader-name "${SHADER_NAME}"
        --source-header-path "${SOURCE_HEADER_PATH}"
        --source-source-path "${SOURCE_SOURCE_PATH}"
        --generated-header-path "${GENERATED_HEADER_PATH}"
        --generated-source-path "${GENERATED_SOURCE_PATH}"

        DEPENDS
        "${RENDERER_SHADER_GENERATE_SCRIPT_PATH}"
        "${SOURCE_HEADER_PATH}"
        "${SOURCE_SOURCE_PATH}"
        "${arg_SHADER_FILE_PATH}"

        # "${PLUGIN_SDK_CORE_DIR_PATH}/__init__.py"
        # "${PLUGIN_SDK_CORE_DIR_PATH}/utils.py"
        # "${PLUGIN_SDK_CORE_DIR_PATH}/datatypes.py"
        # "${PLUGIN_SDK_CORE_DIR_PATH}/parsers/__init__.py"
        # "${PLUGIN_SDK_CORE_DIR_PATH}/parsers/manifest_parse.py"
        # "${PLUGIN_SDK_CORE_DIR_PATH}/generators/__init__.py"
        # "${PLUGIN_SDK_CORE_DIR_PATH}/generators/generate_sdk_files.py"
        COMMENT "Compiling and generating shader code"
        VERBATIM
    )

    set_source_files_properties(
        "${GENERATED_HEADER_PATH}"
        "${GENERATED_SOURCE_PATH}"
        PROPERTIES
        GENERATED TRUE
    )

    target_sources(${TARGET_NAME}
        PRIVATE
        "${GENERATED_HEADER_PATH}"
        "${GENERATED_SOURCE_PATH}"
    )

    target_include_directories(${TARGET_NAME}
        PRIVATE
        "${arg_GEN_DIR_PATH}"
    )
endfunction()
