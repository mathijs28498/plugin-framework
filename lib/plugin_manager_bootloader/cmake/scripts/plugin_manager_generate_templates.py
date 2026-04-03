from pathlib import Path
from plugin_manager_types import *
import textwrap
import json
from dataclasses import asdict


# TODO: Do this in a shared tools python file
def snake_to_pascal_case(snake_str: str):
    return "".join(x.capitalize() for x in snake_str.lower().split("_"))


# TODO: Do this in a shared tools python file
indent_prefix = "    "


# TODO: Do this in a shared tools python file
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


# TODO: Do this in a shared tools python file
def configure_file(
    source_path: Path,
    destination_path: Path,
    replacements: list[tuple[str, str]],
    skip_unchanged_file: bool,
):
    print(f"-- Reading: {source_path}")
    with open(source_path, "r", encoding="utf-8") as f:
        generated_file = f.read()

    for placeholder, actual_value in replacements:
        generated_file = generated_file.replace(f"@{placeholder}@", actual_value)

    create_and_write_to_file(
        destination_path,
        generated_file,
        skip_unchanged_file,
    )


def generate_plugin_manager_interface_declarations(
    source_path: Path,
    destination_path: Path,
    static_plugin_providers: list[PluginProvider],
):
    forward_declarations_list: list[list[str]] = [
        plugin_provider.framework_declarations
        for plugin_provider in static_plugin_providers
    ]

    forward_declarations_text = "\n\n".join(
        "\n".join(forward_declaration for forward_declaration in forward_declarations)
        for forward_declarations in forward_declarations_list
    )

    replacements = [
        ("FORWARD_DECLARATIONS", forward_declarations_text),
    ]

    configure_file(source_path, destination_path, replacements, False)


def generate_plugin_registry_header(
    source_path: Path,
    destination_path: Path,
    interface_definitions: list[InterfaceDefinition],
):
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

    replacements = [
        ("INTERFACE_DEFINITIONS_LEN", str(max(len(interface_definitions), 1))),
        ("PLUGIN_DEFINITIONS_LEN", str(max_plugin_definitions_len)),
        ("PLUGIN_DEPENDENCY_DEFINITIONS_LEN", str(max_plugin_dependencies_len)),
    ]

    configure_file(source_path, destination_path, replacements, False)


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
        raise ValueError(
            "Metadata containing plugin manager not found, please ensure a plugin manager plugin is available"
        )


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


def generate_plugin_manager_cmake(
    source_path: Path,
    destination_path: Path,
    target_name: str,
    include_paths: list[Path],
    source_paths: list[Path],
    plugin_providers: list[PluginProvider],
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
        for plugin_provider in plugin_providers
    )

    plugin_target_cmake_list = f"\n{indent_prefix * 1}".join(
        plugin_provider.plugin_manifest.target_name
        for plugin_provider in plugin_providers
    )

    generated_include_directories_text = f"\n{indent_prefix * 1}".join(
        f'"{include_path.as_posix()}"' for include_path in include_paths
    )

    generated_target_sources_text = f"\n{indent_prefix * 1}".join(
        f'"{source_path.as_posix()}"' for source_path in source_paths
    )

    replacements = [
        ("TARGET_NAME", target_name),
        ("ADD_SUBDIRECTORY_TEXT", add_subdirectory_text),
        ("PLUGIN_TARGET_CMAKE_LIST", plugin_target_cmake_list),
        ("GENERATED_INCLUDE_DIRECTORIES_TEXT", generated_include_directories_text),
        ("GENERATED_TARGET_SOURCES_TEXT", generated_target_sources_text),
    ]

    configure_file(source_path, destination_path, replacements, True)


def generate_statically_resolved_plugin_providers_json(
    destination_path: Path,
    plugin_providers: list[PluginProvider],
):
    plugin_providers_dict = [
        asdict(plugin_provider) for plugin_provider in plugin_providers
    ]

    content = json.dumps(plugin_providers_dict, indent=4, default=str)
    create_and_write_to_file(destination_path, content, False)


def generate_register_inc(
    source_path: Path,
    destination_path: Path,
    build_dynamic: bool,
    plugin_manifest: PluginManifest,
):

    define_build_shared_text = "#define PLUGIN_BUILD_SHARED" if build_dynamic else ""

    interface_name_pascal_case = snake_to_pascal_case(plugin_manifest.interface_name)

    inject_dependency_fn_text = (
        "inject_dependency" if plugin_manifest.dependencies else "NULL"
    )

    inject_dependency_fn_definition_text = ""
    plugin_dependencies_text = ""

    if plugin_manifest.dependencies:
        plugin_dependencies_placeholder = "@PLUGIN_DEPENDENCIES_TEXT@"
        plugin_dependency_text_list: list[str] = []
        inject_dependency_text_list: list[str] = []

        for (
            dependency_interface_name,
            dependency,
        ) in plugin_manifest.dependencies.items():
            inject_dependency_text_list.append(
                textwrap.dedent(
                    f"""\
                    if (strcmp("{dependency_interface_name}", interface_name) == 0)
                    {{
                        context->{dependency.variable_name} = iface;
                        return 0; 
                    }}\
                    """
                )
            )

            plugin_dependency_text_list.append(
                textwrap.dedent(
                    f"""\
                    {{
                        .interface_name = "{dependency_interface_name}",
                    }},\
                    """
                )
            )

        inject_dependency_fn_definition_text = textwrap.dedent(
            f"""\
            static int32_t inject_dependency({interface_name_pascal_case}Context *context, const char *interface_name, void *iface)
            {{
            {plugin_dependencies_placeholder}
                return -1;
            }}\
            """
        )

        inject_dependency_fn_definition_text = (
            inject_dependency_fn_definition_text.replace(
                plugin_dependencies_placeholder,
                textwrap.indent("\n".join(inject_dependency_text_list), indent_prefix),
            )
        )

        plugin_dependencies_text = textwrap.dedent(
            f"""\
            static const PluginDependency plugin_dependencies[] = {{
            {plugin_dependencies_placeholder}
            }};\
            """
        )

        plugin_dependencies_text = plugin_dependencies_text.replace(
            plugin_dependencies_placeholder,
            textwrap.indent("\n".join(plugin_dependency_text_list), indent_prefix),
        )

    # TODO: Make this work with dependencies
    plugin_dependencies_len = len(plugin_manifest.dependencies)
    plugin_dependencies_variable_text = (
        "plugin_dependencies" if plugin_manifest.dependencies else "NULL"
    )

    supported_lifetimes_len = len(plugin_manifest.supported_lifetimes)
    supported_lifetimes_text = f"\n{indent_prefix}".join(
        f"PLUGIN_LIFETIME_{lifetime.name}"
        for lifetime in plugin_manifest.supported_lifetimes
    )
    preferred_lifetime = f"PLUGIN_LIFETIME_{plugin_manifest.preferred_lifetime.name}"

    plugin_init_fn_text = (
        "(PluginInit_Fn)plugin_init" if plugin_manifest.has_init else "NULL"
    )
    plugin_shutdown_fn_text = (
        "(PluginShutdown_Fn)plugin_shutdown" if plugin_manifest.has_shutdown else "NULL"
    )
    interface_name = plugin_manifest.interface_name
    plugin_name = plugin_manifest.plugin_name
    target_name = plugin_manifest.target_name

    replacements = [
        ("DEFINE_BUILD_SHARED_TEXT", define_build_shared_text),
        ("INTERFACE_NAME_PASCAL_CASE", interface_name_pascal_case),
        ("INJECT_DEPENDENCY_FN_DEFINITION_TEXT", inject_dependency_fn_definition_text),
        ("INJECT_DEPENDENCY_FN_TEXT", inject_dependency_fn_text),
        ("PLUGIN_INIT_FN_TEXT", plugin_init_fn_text),
        ("PLUGIN_SHUTDOWN_FN_TEXT", plugin_shutdown_fn_text),
        ("PLUGIN_DEPENDENCIES_TEXT", plugin_dependencies_text),
        ("SUPPORTED_LIFETIMES_TEXT", supported_lifetimes_text),
        ("INTERFACE_NAME", interface_name),
        ("PLUGIN_NAME", plugin_name),
        ("TARGET_NAME", target_name),
        ("PLUGIN_DEPENDENCIES_VARIABLE_TEXT", plugin_dependencies_variable_text),
        ("PLUGIN_DEPENDENCIES_LEN", str(plugin_dependencies_len)),
        ("SUPPORTED_LIFETIMES_LEN", str(supported_lifetimes_len)),
        ("PREFERRED_LIFETIME", preferred_lifetime),
    ]

    configure_file(source_path, destination_path, replacements, False)


def generate_plugin_dependencies(
    source_path: Path,
    destination_path: Path,
    plugin_manifest: PluginManifest,
):
    dependency_interface_forward_declarations_text = "\n".join(
        f"struct {snake_to_pascal_case(dependency_interface_name)}Interface;"
        for dependency_interface_name in plugin_manifest.dependencies.keys()
    )

    dependency_variables_text = ""

    if plugin_manifest.dependencies:
        define_start = textwrap.dedent(
            f""" \
            \\
            union {{ \\
                struct {{ \\
            """
        )
        define_middle = textwrap.indent(
            f" \\\n{indent_prefix}".join(
                f"struct {snake_to_pascal_case(dependency_interface_name)}Interface *{dependency.variable_name};"
                for dependency_interface_name, dependency in plugin_manifest.dependencies.items()
            ),
            indent_prefix * 3,
        )
        define_end = textwrap.dedent(
            f""" \
            \\
                }}; \\
                void *interfaces[{str(len(plugin_manifest.dependencies))}]; \\
            }};
            """
        )

        dependency_variables_text = define_start + define_middle + define_end

    # union { \
    #     struct { \
    #     };\
    #     void *interfaces[@DEPENDENCY_VARIABLES_LEN@]; \
    # };
    replacements = [
        (
            "DEPENDENCY_INTERFACE_FORWARD_DECLARATIONS_TEXT",
            dependency_interface_forward_declarations_text,
        ),
        ("DEPENDENCY_VARIABLES_TEXT", dependency_variables_text),
    ]

    configure_file(source_path, destination_path, replacements, False)
