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
    internal_plugins: list[InternalPlugin],
):
    forward_declarations = "\n\n".join(
        "\n".join(internal_plugin.forward_declarations)
        for internal_plugin in internal_plugins
    )
    replacements = {
        "FORWARD_DECLARATIONS": forward_declarations,
    }

    configure_file(source_path, destination_path, replacements, False)


def generate_get_setup_context_src(
    source_path: Path,
    destination_path: Path,
    internal_plugins: list[InternalPlugin],
    plugin_list_data: PluginListData,
):
    internal_plugins_text = f"{{0}}"
    requested_plugins_text = f"{{0}}"
    sorted_plugin_providers_indices_text = f"{{0}}"
    plugin_providers_text = f"{{0}}"

    if len(internal_plugins):
        internal_plugins_text = ",\n".join(
            textwrap.dedent(
                f"""\
                {{
                    .interface_name = "{internal_plugin.interface_name}",
                    .plugin_name = "{internal_plugin.plugin_name}",
                    .dependencies = {{0}},
                    .dependencies_len = {str(len(internal_plugin.dependencies))},
                    .get_interface = {internal_plugin.get_interface_fn_text},
                    .init = {internal_plugin.init_fn_text},
                    .shutdown = {internal_plugin.shutdown_fn_text},
                    .is_explicit = {str(internal_plugin.is_explicit).lower()},
                    .is_initialized = {str(internal_plugin.is_initialized).lower()},
                }}"""
            )
            for internal_plugin in internal_plugins
        )

        internal_plugins_text = textwrap.indent(
            f"\n{internal_plugins_text},\n{indent_prefix * 2}",
            indent_prefix * 3,
        )

    if len(plugin_list_data.requested_plugins):
        requested_plugins_text = ",\n".join(
            textwrap.dedent(
                f"""\
                {{
                    .interface_name = "{requested_plugin.interface_name}",
                    .plugin_name = "{requested_plugin.plugin_name or ""}",
                    .is_explicit = {str(requested_plugin.is_explicit).lower()},
                    .is_resolved = {str(requested_plugin.is_resolved).lower()},
                }}"""
            )
            for requested_plugin in plugin_list_data.requested_plugins
        )

        requested_plugins_text = textwrap.indent(
            f"\n{requested_plugins_text},\n{indent_prefix * 2}",
            indent_prefix * 3,
        )

    # TODO: Make this work with plugin providers and stuff
    replacements = {
        "INTERNAL_PLUGINS_TEXT": internal_plugins_text,
        "INTERNAL_PLUGINS_LEN": str(len(internal_plugins)),
        "REQUESTED_PLUGINS_TEXT": requested_plugins_text,
        "REQUESTED_PLUGINS_LEN": str(len(plugin_list_data.requested_plugins)),
        "SORTED_PLUGIN_PROVIDERS_INDICES_TEXT": sorted_plugin_providers_indices_text,
        "PLUGIN_PROVIDERS_TEXT": plugin_providers_text,
        "PLUGIN_PROVIDERS_LEN": str(len(plugin_list_data.plugin_providers)),
    }

    configure_file(source_path, destination_path, replacements, False)


def generate_plugin_registry_src(
    source_path: Path,
    destination_path: Path,
    interface_definitions: list[InterfaceDefinition],
):
    interface_definitions_text = f"{{0}}"
    if len(interface_definitions) > 0:
        interface_definition_text_list: list[str] = []
        for interface_definition in interface_definitions:
            plugin_definitions_text = f"{{0}}"
            if len(interface_definition.plugin_definitions) > 0:
                plugin_definitions_text = ",\n".join(
                    textwrap.dedent(
                        f"""\
                        {{
                            .plugin_name = "{plugin.plugin_name}",
                            .module_path = "{plugin.module_path.as_posix()}",
                        }}"""
                    )
                    for plugin in interface_definition.plugin_definitions
                )
                plugin_definitions_text = textwrap.indent(
                    f"\n{plugin_definitions_text},\n{indent_prefix * 1}",
                    indent_prefix * 2,
                )

            plugin_definition_placeholder = "@PLUGIN_DEFINITIONS_TEXT@"
            interface_definition_text = textwrap.dedent(
                f"""\
                {{
                    .interface_name = "{interface_definition.interface_name}",
                    .default_plugin = "{interface_definition.default_plugin_name}",
                    .plugin_definitions = {{{plugin_definition_placeholder}}},
                    .plugin_definitions_len = {len(interface_definition.plugin_definitions)},
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


def generate_plugin_registry_header(
    source_path: Path,
    destination_path: Path,
    interface_definitions: list[InterfaceDefinition],
):
    replacements = {
        "INTERFACE_DEFINITIONS_LEN": str(len(interface_definitions)),
        "PLUGIN_DEFINITIONS_LEN": str(
            max(
                (
                    len(interface_definition.plugin_definitions)
                    for interface_definition in interface_definitions
                ),
                default=0,
            )
        ),
    }

    configure_file(source_path, destination_path, replacements, False)


def generate_plugin_manager_cmake(
    source_path: Path,
    destination_path: Path,
    target_name: str,
    generated_get_setup_context_src_path: Path,
    generated_plugin_registry_src_path: Path,
    internal_plugins: list[InternalPlugin],
):
    # TODO: Add loops in the template
    add_subdirectory_text = "\n\n".join(
        textwrap.dedent(
            f"""\
            add_subdirectory(
                "{internal_plugin.source_data.source_path.as_posix()}"
                "${{CMAKE_CURRENT_BINARY_DIR}}/generated_plugins/{internal_plugin.source_data.target_name}"
            )"""
        )
        for internal_plugin in internal_plugins
    )

    plugin_target_cmake_list = f"\n{indent_prefix * 2}".join(
        internal_plugin.source_data.target_name for internal_plugin in internal_plugins
    )

    replacements = {
        "TARGET_NAME": target_name,
        "ADD_SUBDIRECTORY_TEXT": add_subdirectory_text,
        "PLUGIN_TARGET_CMAKE_LIST": plugin_target_cmake_list,
        "GENERATED_GET_SETUP_CONTEXT_SRC_PATH": generated_get_setup_context_src_path.as_posix(),
        "GENERATED_PLUGIN_REGISTRY_SRC_PATH": generated_plugin_registry_src_path.as_posix(),
    }

    configure_file(source_path, destination_path, replacements, True)
