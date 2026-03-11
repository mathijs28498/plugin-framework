from plugin_manager_generate_templates import *
from plugin_manager_types import *
from plugin_manager_parse import *
from plugin_manager_generate_arguments import parse_cmake_arguments
from plugin_manager_plugin_resolver import create_static_plugin_providers


def main():
    arguments = parse_cmake_arguments()

    plugin_registry_dict = read_toml(arguments.plugin_registry_toml)
    plugin_registry = parse_plugin_registry(
        plugin_registry_dict, arguments.build_platform
    )

    static_plugin_providers = create_static_plugin_providers(
        plugin_registry, arguments.build_dynamic_plugins
    )

    # TODO: Create all providers and do dependencies if arguments.build_dynamic_plugins is False

    # if not arguments.dynamic_plugins:
    #     plugin_list_json = read_json(arguments.plugin_list_json)

    #     requested_plugins = parse_plugin_list(plugin_list_json)
    #     interface_definitions = parse_plugin_registry(
    #         plugin_registry_json, arguments.build_platform
    #     )

    #     for requested_plugin in requested_plugins:
    #         interface_definition = next(
    #             interface_definition
    #             for interface_definition in interface_definitions
    #             if interface_definition.interface_name
    #             == requested_plugin.interface_name
    #         )

    #         if not interface_definition :
    #             raise ValueError(
    #                 f"Interface definition '{requested_plugin.interface_name}' not found"
    #             )

    #         plugin_name = (
    #             interface_definition.default_plugin_name
    #             if requested_plugin.plugin_name == ""
    #             else requested_plugin.plugin_name
    #         )

    #         plugin_definition = next(
    #             plugin_definition
    #             for plugin_definition in interface_definition.plugin_definitions
    #             if plugin_definition.plugin_name == plugin_name
    #         )

    #         if not plugin_definition :
    #             raise ValueError(
    #                 f"Plugin definition '{requested_plugin.interface_name}' '{plugin_name}' not found"
    #             )

    #         plugin_register_path = Path(
    #             f"{plugin_definition.source_path}/{requested_plugin.interface_name}_{plugin_name}_register.c"
    #         )
    #         print(f"-- Reading: {plugin_register_path}")
    #         with open(plugin_register_path, "r", encoding="utf-8") as f:
    #             plugin_register_source = f.read()
    #             # TODO: Do this better!
    #             dependencies_match = re.search(
    #                 r"^#define PLUGIN_DEPENDENCIES.*\n",
    #                 plugin_register_source,
    #                 re.MULTILINE,
    #             )
    # if dependencies_match :
    #     dependencies_end = dependencies_match.end()
    #     interface_dependencies = [
    #         re.search(r"\((.*)\)", line).group(1).split(",")[2].strip()
    #         for line in plugin_register_source[dependencies_end:].split(
    #             "\n"
    #         )
    #         if re.match(r"[    |\t]*X\(.*\)", line)
    #     ]

    #     print(interface_dependencies)

    """
    TODO: Configure time:
    - Create json for registering plugins
    - Generate register code into .inc file based on json using plugin_sdk.h
    - Treat internal plugins just like statically build plugins
    - Use json to determine everything necessary
    - Write code to initialize these internal plugins before the program starts (figure out how)
    - Check requested plugins
    - Resolve requested plugins
    - Check dependencies of these plugins
    - Add dependencies to requested plugins and repeat until no more
    - Save these plugins with their dependencies to json for compile time
        - Make sure the is_explicit is only true for the original requested plugins
    """

    generate_plugin_manager_cmake(
        arguments.source_cmake,
        arguments.generated_cmake,
        arguments.target_name,
        [
            arguments.generated_include_dir,
        ],
        [
            arguments.generated_plugin_registry_src,
            arguments.generated_init_contexts_src,
        ],
        static_plugin_providers,
    )


if __name__ == "__main__":
    main()
