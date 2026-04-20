from pathlib import Path

import textwrap

from plugin_sdk_core.datatypes import PluginManifest
from plugin_sdk_core.utils import snake_to_pascal_case, indent_prefix, configure_file

def generate_register_inc(
    source_path: Path,
    destination_path: Path,
    build_dynamic: bool,
    plugin_manifest: PluginManifest,
):
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
    dependency_interface_forward_declarations_text_list = [
        "\nstruct ScopedPluginInterface;"
    ]
    dependency_interface_forward_declarations_text_list.extend(
        f"struct {snake_to_pascal_case(dependency_interface_name)}Interface;"
        for dependency_interface_name in plugin_manifest.dependencies.keys()
    )

    dependency_interface_forward_declarations_text = "\n".join(
        dependency_interface_forward_declarations_text_list
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
                struct ScopedPluginInterface *interfaces[{str(len(plugin_manifest.dependencies))}]; \\
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
