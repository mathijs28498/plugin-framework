from plugin_manager_generate_templates import *
from plugin_manager_parse import *
from plugin_manager_generate_arguments import parce_c_code_arguments
from plugin_manager_plugin_resolver import *
from plugin_manager_generate_init_contexts import generate_init_contexts_src

RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH = 256


def main():
    arguments = parce_c_code_arguments()

    plugin_registry_dict = read_toml(arguments.plugin_registry_toml)
    requested_plugins_dict = read_toml(arguments.requested_plugins_toml)
    statically_resolved_plugins_dict = read_json(
        arguments.statically_resolved_plugins_json
    )

    plugin_registry = parse_plugin_registry(
        plugin_registry_dict,
        arguments.build_platform,
    )
    requested_plugins: list[RequestedPlugin] = []
    if arguments.build_dynamic_plugins:
        requested_plugins = parse_requested_plugins(requested_plugins_dict)

    plugin_providers = list(
        parse_statically_resolved_plugins(statically_resolved_plugins_dict)
    )

    interface_definitions = create_interface_definitions(plugin_registry)
    plugin_providers = sort_plugin_providers(plugin_providers)

    generate_plugin_registry_header(
        arguments.source_plugin_registry_header,
        arguments.generated_plugin_registry_header,
        interface_definitions,
    )

    generate_plugin_manager_interface_declarations(
        arguments.source_plugin_manager_interface_declarations,
        arguments.generated_plugin_manager_interface_declarations,
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
    )


if __name__ == "__main__":
    main()
