from pathlib import Path
from plugin_manager_types import *
import textwrap

indent_prefix = "    "


def create_and_write_to_file(
    destination_path: Path,
    content: str,
    skip_unchanged_file: bool,
):
    if skip_unchanged_file and destination_path.exists():
        with open(destination_path, "r", encoding="utf-8") as f:
            if f.read() == content:
                print(
                    f"-- Skip writing as new content is the same as old: {destination_path}"
                )
                return
    print(f"-- Creating and writing to: {destination_path}")
    destination_path.parent.mkdir(exist_ok=True, parents=True)

    with open(destination_path, "w", encoding="utf-8") as f:
        f.write(content)


def configure_file(
    source_path: Path,
    destination_path: Path,
    replacements: dict[str, str],
    skip_unchanged_file: bool,
):
    print(f"-- Reading: {source_path}")
    with open(source_path, "r", encoding="utf-8") as f:
        generated_file = f.read()

    for placeholder, actual_value in replacements.items():
        generated_file = generated_file.replace(f"@{placeholder}@", actual_value)

    create_and_write_to_file(
        destination_path,
        generated_file,
        skip_unchanged_file,
    )


def generate_plugin_manager_header(
    source_path: Path,
    destination_path: Path,
    static_plugin_providers: list[PluginProvider],
):
    forward_declarations_list: list[list[str]] = [
        plugin_provider.plugin_manifest.exported_declarations
        + plugin_provider.framework_declarations
        for plugin_provider in static_plugin_providers
    ]

    forward_declarations_text = "\n\n".join(
        "\n".join(forward_declaration for forward_declaration in forward_declarations)
        for forward_declarations in forward_declarations_list
    )

    replacements = {
        "FORWARD_DECLARATIONS": forward_declarations_text,
    }

    configure_file(source_path, destination_path, replacements, False)


def generate_plugin_registry_header(
    source_path: Path,
    destination_path: Path,
    interface_definitions: list[InterfaceDefinition],
):
    # TODO: Figure out how to do this properly
    max_plugin_definitions_len = max(
        max(
            (
                len(interface_definition.plugin_manifests)
                for interface_definition in interface_definitions
            ),
            default=1,
        ),
        1,
    )

    max_plugin_dependencies_len = max(
        max(
            (
                max(
                    len(plugin_manifest.dependencies)
                    for plugin_manifest in interface_definition.plugin_manifests
                )
                for interface_definition in interface_definitions
            ),
            default=1,
        ),
        1,
    )

    replacements = {
        "INTERFACE_DEFINITIONS_LEN": str(max(len(interface_definitions), 1)),
        "PLUGIN_DEFINITIONS_LEN": str(max_plugin_definitions_len),
        "PLUGIN_DEPENDENCY_DEFINITIONS_LEN": str(max_plugin_dependencies_len),
    }

    configure_file(source_path, destination_path, replacements, False)


def generate_plugin_registry_src(
    source_path: Path,
    destination_path: Path,
    interface_definitions: list[InterfaceDefinition],
):
    interface_definitions_text = f"{{0}}"
    if interface_definitions:
        interface_definition_text_list: list[str] = []
        for interface_definition in interface_definitions:
            plugin_definitions_text = f"{{{{0}}}}"
            if interface_definition.plugin_manifests:
                plugin_definitions_text_list: list[str] = []
                for manifest in interface_definition.plugin_manifests:

                    plugin_dependencies_text = f"{{0}}"
                    if manifest.dependencies:
                        dependency_text_list: list[str] = []
                        for dependency in manifest.dependencies:
                            dependency_text_list.append(
                                textwrap.dedent(
                                    f"""\
                                    {{
                                        .interface_name = "{dependency.interface_name}",
                                    }}"""
                                )
                            )

                        dependency_definitions_text = ",\n".join(dependency_text_list)
                        plugin_dependencies_text = textwrap.indent(
                            f"\n{dependency_definitions_text},\n{indent_prefix * 1}",
                            indent_prefix * 2,
                        )

                    dependency_placeholder = "@DEPENDENCY_DEFINITIONS_TEXT@"
                    plugin_definition_text = textwrap.dedent(
                        f"""\
                        {{
                            .interface_name = "{manifest.interface_name}",
                            .target_name = "{manifest.target_name}",
                            .plugin_name = "{manifest.plugin_name}",
                            .dependencies = {{{dependency_placeholder}}},
                            .dependencies_len = {str(len(manifest.dependencies))},
                            .has_init = {"true" if manifest.init_fn else "false"},
                            .has_shutdown = {"true" if manifest.shutdown_fn else "false"},
                            .is_static_only = {str(manifest.static_only).lower()},
                            .module_path = {f'"{manifest.module_path.as_posix()}"' if manifest.module_path is not None else "NULL"},
                        }}"""
                    )
                    plugin_definition_text = plugin_definition_text.replace(
                        dependency_placeholder,
                        plugin_dependencies_text,
                    )
                    plugin_definitions_text_list.append(plugin_definition_text)

                plugin_definitions_text = ",\n".join(plugin_definitions_text_list)
                plugin_definitions_text = textwrap.indent(
                    f"\n{plugin_definitions_text},\n{indent_prefix * 1}",
                    indent_prefix * 2,
                )

            plugin_definition_placeholder = "@PLUGIN_DEFINITIONS_TEXT@"
            interface_definition_text = textwrap.dedent(
                f"""\
                {{
                    .interface_name = "{interface_definition.interface_name}",
                    .default_plugin = "{interface_definition.default_plugin_manifest.plugin_name}",
                    .plugin_definitions = {{{plugin_definition_placeholder}}},
                    .plugin_definitions_len = {str(len(interface_definition.plugin_manifests))},
                }}"""
            )
            interface_definition_text = interface_definition_text.replace(
                plugin_definition_placeholder,
                plugin_definitions_text,
            )
            interface_definition_text_list.append(interface_definition_text)

        interface_definitions_text = ",\n".join(interface_definition_text_list)
        interface_definitions_text = textwrap.indent(
            f"\n{interface_definitions_text},\n{indent_prefix * 2}",
            indent_prefix * 3,
        )

    replacements = {
        "INTERFACE_DEFINITIONS_TEXT": f"{interface_definitions_text}",
        "INTERFACE_DEFINITIONS_LEN": str(len(interface_definitions)),
    }
    configure_file(source_path, destination_path, replacements, False)


def generate_plugin_manager_cmake(
    source_path: Path,
    destination_path: Path,
    target_name: str,
    include_paths: list[Path],
    source_paths: list[Path],
    static_plugin_providers: list[PluginProvider],
):
    # TODO: Add loops in the template
    add_subdirectory_text = "\n\n".join(
        textwrap.dedent(
            f"""\
            add_subdirectory(
                "{plugin_provider.plugin_manifest.source_path.as_posix()}"
                "${{CMAKE_CURRENT_BINARY_DIR}}/generated_plugins/{plugin_provider.plugin_manifest.target_name}"
            )"""
        )
        for plugin_provider in static_plugin_providers
    )

    plugin_target_cmake_list = f"\n{indent_prefix * 1}".join(
        plugin_provider.plugin_manifest.target_name
        for plugin_provider in static_plugin_providers
    )

    generated_include_directories_text = f"\n{indent_prefix * 1}".join(
        f'"{include_path.as_posix()}"' for include_path in include_paths
    )

    generated_target_sources_text = f"\n{indent_prefix * 1}".join(
        f'"{source_path.as_posix()}"' for source_path in source_paths
    )

    replacements = {
        "TARGET_NAME": target_name,
        "ADD_SUBDIRECTORY_TEXT": add_subdirectory_text,
        "PLUGIN_TARGET_CMAKE_LIST": plugin_target_cmake_list,
        "GENERATED_INCLUDE_DIRECTORIES_TEXT": generated_include_directories_text,
        "GENERATED_TARGET_SOURCES_TEXT": generated_target_sources_text,
    }

    configure_file(source_path, destination_path, replacements, True)


def generate_register_inc(
    source_path: Path,
    destination_path: Path,
    build_dynamic: bool,
    plugin_manifest: PluginManifest,
):

    define_plugin_target_name_text = (
        f"#define PLUGIN_TARGET_NAME {plugin_manifest.target_name}"
    )

    define_build_shared_text = ""
    if not plugin_manifest.static_only and build_dynamic:
        define_build_shared_text = "#define PLUGIN_BUILD_SHARED"

    interfacePascalCase = (
        plugin_manifest.interface_name.replace("_", " ").title().replace(" ", "")
    )

    register_macros_text_list = [
        f"PLUGIN_REGISTER_INTERFACE({plugin_manifest.get_interface_fn})"
    ]

    if plugin_manifest.init_fn:
        register_macros_text_list.append(
            f"PLUGIN_REGISTER_INIT({plugin_manifest.init_fn})"
        )

    if plugin_manifest.shutdown_fn:
        register_macros_text_list.append(
            f"PLUGIN_REGISTER_SHUTDOWN({plugin_manifest.shutdown_fn})"
        )

    if plugin_manifest.dependencies:
        register_macros_text_list.append(
            f"\n".join(
                f"PLUGIN_REGISTER_DEPENDENCY({interfacePascalCase}InterfaceContext, {dependency.variable_name}, {dependency.interface_name})"
                for dependency in plugin_manifest.dependencies
            )
        )

    replacements = {
        "DEFINE_BUILD_SHARED_TEXT": define_build_shared_text,
        "DEFINE_PLUGIN_TARGET_NAME_TEXT": define_plugin_target_name_text,
        "REGISTER_MACROS_TEXT": "\n\n".join(register_macros_text_list),
    }

    configure_file(source_path, destination_path, replacements, False)
