from plugin_manager_generate_templates import *
from plugin_manager_parse import *
from plugin_manager_generate_arguments import parce_c_code_arguments
from plugin_manager_plugin_resolver import (
    create_static_plugin_providers,
    create_interface_definitions,
)
from plugin_manager_generate_init_contexts import generate_init_contexts_src


def main():
    arguments = parce_c_code_arguments()

    plugin_registry_dict = read_toml(arguments.plugin_registry_toml)
    plugin_list_dict = read_toml(arguments.plugin_list_toml)

    plugin_registry = parse_plugin_registry(
        plugin_registry_dict,
        arguments.build_platform,
    )
    requested_plugins = parse_plugin_list(plugin_list_dict)

    plugin_providers = create_static_plugin_providers(
        plugin_registry,
        arguments.build_dynamic_plugins,
    )
    interface_definitions = create_interface_definitions(plugin_registry)
    interface_instances = []

    if not arguments.build_dynamic_plugins:
        requested_plugins = []
        interface_definitions = []

    #     TODO: Configure time:
    #     - Check requested plugins
    #     - Resolve requested plugins
    #     - Check dependencies of these plugins
    #     - Add dependencies to requested plugins and repeat until no more
    #     - Save these plugins with their dependencies to json for compile time
    #         - Make sure the is_explicit is only true for the original requested plugins

    #     TODO: Compile time:
    #     - Read inserted plugins created by configure
    #     - Read registered methods
    #         - This now happens for InternalPlugins, make sure this becomes 1 method
    #     - Create plugin_providers list
    #     - Do topological sort to figure out sorting order
    #     - Put data into PluginListData
    #         - Think of better name for PluginListData
    #     - Make sure c file gets properly created
    #     """

    generate_plugin_registry_header(
        arguments.source_plugin_registry_header,
        arguments.generated_plugin_registry_header,
        interface_definitions,
    )

    generate_plugin_manager_header(
        arguments.source_plugin_manager_header,
        arguments.generated_plugin_manager_header,
        plugin_providers,
    )

    generate_plugin_registry_src(
        arguments.source_plugin_registry_src,
        arguments.generated_plugin_registry_src,
        interface_definitions,
    )

    generate_init_contexts_src(
        arguments.source_init_contexts_src,
        arguments.generated_init_contexts_src,
        requested_plugins,
        # The plugin_providers list is assumed to be properly sorted already, so sorted plugin indices can be created like this
        [i for i in range(len(plugin_providers))],
        plugin_providers,
        interface_instances,
    )


if __name__ == "__main__":
    main()
