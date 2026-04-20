from pathlib import Path
from internal_core.datatypes import InterfaceDefinition, PluginRegistry, RequestedPlugin
from plugin_sdk_core.utils import (
    indent_prefix,
    create_and_write_to_file,
    configure_file,
)
from plugin_sdk_core.datatypes import PluginManifest
import textwrap


# def generate_plugin_manager_interface_declarations(
#     source_path: Path,
#     destination_path: Path,
#     static_plugin_providers: list[PluginProvider],
# ):
#     forward_declarations_list: list[list[str]] = [
#         plugin_provider.framework_declarations
#         for plugin_provider in static_plugin_providers
#     ]

#     forward_declarations_text = "\n\n".join(
#         "\n".join(forward_declaration for forward_declaration in forward_declarations)
#         for forward_declarations in forward_declarations_list
#     )

#     replacements = [
#         ("FORWARD_DECLARATIONS", forward_declarations_text),
#     ]

#     configure_file(source_path, destination_path, replacements, False)


# def generate_plugin_registry_header(
#     source_path: Path,
#     destination_path: Path,
#     interface_definitions: list[InterfaceDefinition],
# ):
#     max_plugin_definitions_len = max(
#         max(
#             (
#                 len(interface_definition.plugin_manifests)
#                 for interface_definition in interface_definitions
#             ),
#             default=1,
#         ),
#         1,
#     )

#     max_plugin_dependencies_len = max(
#         max(
#             (
#                 max(
#                     len(plugin_manifest.dependencies)
#                     for plugin_manifest in interface_definition.plugin_manifests
#                 )
#                 for interface_definition in interface_definitions
#             ),
#             default=1,
#         ),
#         1,
#     )

#     replacements = [
#         ("INTERFACE_DEFINITIONS_LEN", str(max(len(interface_definitions), 1))),
#         ("PLUGIN_DEFINITIONS_LEN", str(max_plugin_definitions_len)),
#         ("PLUGIN_DEPENDENCY_DEFINITIONS_LEN", str(max_plugin_dependencies_len)),
#     ]

#     configure_file(source_path, destination_path, replacements, False)


def create_plugin_metadata_replacements(
    plugin_manifests: list[PluginManifest],
) -> list[tuple[str, str]]:
    # TODO: figure out what the argument should hold
    get_plugin_metadata_forward_declarations = "\n".join(
        f"extern const PluginMetadata {plugin_manifest.target_name}_plugin_metadata;"
        for plugin_manifest in plugin_manifests
    )
    plugin_metadatas_text = textwrap.indent(
        ",\n".join(
            f"&{plugin_manifest.target_name}_plugin_metadata"
            for plugin_manifest in plugin_manifests
        ),
        indent_prefix * 1,
    )
    plugin_metadatas_len = len(plugin_manifests)
    plugin_manager_metadata_text = next(
        (
            f"&{plugin_manifest.target_name}_plugin_metadata"
            for plugin_manifest in plugin_manifests
            if plugin_manifest.interface_name == "plugin_manager"
        ),
        None,
    )
    if plugin_manager_metadata_text == None:
        plugin_manager_metadata_text = "NULL"

    return [
        (
            "GET_PLUGIN_METADATA_FORWARD_DECLARATIONS",
            get_plugin_metadata_forward_declarations,
        ),
        ("PLUGIN_METADATAS_TEXT", plugin_metadatas_text),
        ("PLUGIN_MANAGER_METADATA_TEXT", plugin_manager_metadata_text),
    ]


def create_plugin_registry_replacements(
    interface_definitions: list[InterfaceDefinition],
) -> list[tuple[str, str]]:
    plugin_dependencies_text_list: list[str] = []
    plugin_definitions_text_list: list[str] = []
    interface_definitions_text_list: list[str] = []

    for interface_definition in interface_definitions:
        plugin_definitions_array_name = (
            f"{interface_definition.interface_name}_plugin_definitions"
        )
        interface_definitions_text_list.append(
            textwrap.dedent(
                f"""\
                {{
                    .interface_name = "{interface_definition.interface_name}",
                    .default_plugin = "{interface_definition.default_plugin_manifest.plugin_name}",
                    .plugin_definitions = {plugin_definitions_array_name},
                    .plugin_definitions_len = {str(len(interface_definition.plugin_manifests))},
                }},\
                """
            )
        )

        plugin_definition_text_list: list[str] = []

        for plugin_manifest in interface_definition.plugin_manifests:
            plugin_dependencies_array_name = (
                (f"{plugin_manifest.target_name}_dependencies")
                if plugin_manifest.dependencies
                else "NULL"
            )

            plugin_definition_text_list.append(
                textwrap.dedent(
                    f"""\
                    {{
                        .target_name = "{plugin_manifest.target_name}",
                        .interface_name = "{plugin_manifest.interface_name}",
                        .plugin_name = "{plugin_manifest.plugin_name}",
                        .dependencies = {plugin_dependencies_array_name},
                        .dependencies_len = {str(len(plugin_manifest.dependencies))},
                        .has_init = {str(plugin_manifest.has_init).lower()},
                        .has_shutdown = {str(plugin_manifest.has_shutdown).lower()},
                        .module_path = "{plugin_manifest.module_path.as_posix()}",
                    }},\
                    """
                )
            )

            if not plugin_manifest.dependencies:
                break

            plugin_dependency_text_list: list[str] = []

            for (
                dependency_interface_name,
                dependency,
            ) in plugin_manifest.dependencies.items():
                plugin_dependency_text_list.append(
                    textwrap.dedent(
                        f"""\
                        {{
                            .interface_name = "{dependency_interface_name}",
                            .is_deferred = {str(dependency.is_deferred).lower()},
                        }},\
                        """
                    )
                )

            plugin_dependencies_placeholder = "@PLUGIN_DEPENDENCIES_TEXT@"
            plugin_dependencies_text = textwrap.dedent(
                f"""\
                static const PluginDependencyDefinition {plugin_dependencies_array_name}[] = {{
                {plugin_dependencies_placeholder}
                }};"""
            )

            plugin_dependencies_text = plugin_dependencies_text.replace(
                plugin_dependencies_placeholder,
                textwrap.indent(
                    "\n".join(plugin_dependency_text_list), indent_prefix * 1
                ),
            )
            plugin_dependencies_text_list.append(plugin_dependencies_text)

        plugin_definitions_placeholder = "@PLUGIN_DEFINITIONS_TEXT@"
        plugin_definitions_text = textwrap.dedent(
            f"""\
            static const PluginDefinition {plugin_definitions_array_name}[] = {{
            {plugin_definitions_placeholder}
            }};"""
        )

        plugin_definitions_text = plugin_definitions_text.replace(
            plugin_definitions_placeholder,
            textwrap.indent("\n".join(plugin_definition_text_list), indent_prefix * 1),
        )
        plugin_definitions_text_list.append(plugin_definitions_text)

    plugin_dependencies_text = "\n\n".join(plugin_dependencies_text_list)
    plugin_definitions_text = "\n\n".join(plugin_definitions_text_list)
    interface_definitions_text = textwrap.indent(
        f"\n".join(interface_definitions_text_list), indent_prefix * 1
    )

    return [
        ("PLUGIN_DEPENDENCIES_TEXT", plugin_dependencies_text),
        ("PLUGIN_DEFINITIONS_TEXT", plugin_definitions_text),
        ("INTERFACE_DEFINITIONS_TEXT", interface_definitions_text),
        ("INTERFACE_DEFINITIONS_LEN", str(len(interface_definitions))),
    ]


def create_requested_plugin_replacements(
    requested_plugins: list[RequestedPlugin],
) -> list[tuple[str, str]]:
    requested_plugins_text = textwrap.indent(
        "\n".join(
            textwrap.dedent(
                f"""\
                {{
                    .interface_name = "{requested_plugin.interface_name}",    
                    .plugin_name = "{requested_plugin.plugin_name if requested_plugin.plugin_name else ""}",
                    .lifetime = PLUGIN_LIFETIME_{requested_plugin.lifetime.name},
                }},\
                """
            )
            for requested_plugin in requested_plugins
        ),
        indent_prefix,
    )

    return [
        ("REQUESTED_PLUGINS_TEXT", requested_plugins_text),
        ("REQUESTED_PLUGINS_LEN", str(len(requested_plugins))),
    ]


def generate_plugin_manager_bootloader_generated_src(
    source_path: Path,
    destination_path: Path,
    interface_definitions: list[InterfaceDefinition],
    plugin_manifests: list[PluginManifest],
    requested_plugins: list[RequestedPlugin],
):
    replacements = (
        create_plugin_registry_replacements(interface_definitions)
        + create_plugin_metadata_replacements(plugin_manifests)
        + create_requested_plugin_replacements(requested_plugins)
    )

    configure_file(source_path, destination_path, replacements, False)


def generate_plugin_manager_depfile(
    destination_path: Path,
    generated_src_path: Path,
    plugin_registry: PluginRegistry,
):
    manifest_paths_content = " \\\n  ".join(
        manifest.manifest_path.as_posix()
        for manifest in plugin_registry.plugin_manifests
    )
    manifest_paths_content = (
        f"{generated_src_path.as_posix()}: \\\n  {manifest_paths_content}\n"
    )

    create_and_write_to_file(destination_path, manifest_paths_content, False)


# def generate_plugin_dependencies(
#     source_path: Path,
#     destination_path: Path,
#     plugin_manifest: PluginManifest,
# ):
#     dependency_interface_forward_declarations_text_list = [
#         "\nstruct ScopedPluginInterface;"
#     ]
#     dependency_interface_forward_declarations_text_list.extend(
#         f"struct {snake_to_pascal_case(dependency_interface_name)}Interface;"
#         for dependency_interface_name in plugin_manifest.dependencies.keys()
#     )

#     dependency_interface_forward_declarations_text = "\n".join(
#         dependency_interface_forward_declarations_text_list
#     )

#     dependency_variables_text = ""

#     if plugin_manifest.dependencies:
#         define_start = textwrap.dedent(
#             f""" \
#             \\
#             union {{ \\
#                 struct {{ \\
#             """
#         )
#         define_middle = textwrap.indent(
#             f" \\\n{indent_prefix}".join(
#                 f"struct {snake_to_pascal_case(dependency_interface_name)}Interface *{dependency.variable_name};"
#                 for dependency_interface_name, dependency in plugin_manifest.dependencies.items()
#             ),
#             indent_prefix * 3,
#         )
#         define_end = textwrap.dedent(
#             f""" \
#             \\
#                 }}; \\
#                 struct ScopedPluginInterface *interfaces[{str(len(plugin_manifest.dependencies))}]; \\
#             }};
#             """
#         )

#         dependency_variables_text = define_start + define_middle + define_end

#     replacements = [
#         (
#             "DEPENDENCY_INTERFACE_FORWARD_DECLARATIONS_TEXT",
#             dependency_interface_forward_declarations_text,
#         ),
#         ("DEPENDENCY_VARIABLES_TEXT", dependency_variables_text),
#     ]

#     configure_file(source_path, destination_path, replacements, False)
