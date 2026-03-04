from plugin_manager_generate_templates import *
from plugin_manager_parse_json import *
from plugin_manager_generate_arguments import parce_c_code_arguments

def main():
    arguments = parce_c_code_arguments()

    plugin_registry_json = read_json(arguments.plugin_registry_json)
    plugin_list_json = read_json(arguments.plugin_list_json)

    internal_plugins = parse_internal_plugins(plugin_registry_json)
    plugin_list_data = parse_plugin_list(plugin_list_json)
    interface_definitions = parse_plugin_registry(
        plugin_registry_json, arguments.build_platform
    )

    generate_plugin_registry_header(
        arguments.source_plugin_registry_header,
        arguments.generated_plugin_registry_header,
        interface_definitions,
    )

    generate_plugin_manager_header(
        arguments.source_plugin_manager_header,
        arguments.generated_plugin_manager_header,
        internal_plugins,
    )

    generate_plugin_registry_src(
        arguments.source_plugin_registry_src,
        arguments.generated_plugin_registry_src,
        interface_definitions,
    )

    generate_get_setup_context_src(
        arguments.source_get_setup_context_src,
        arguments.generated_get_setup_context_src,
        internal_plugins,
        plugin_list_data,
    )

if __name__ == "__main__":
    main()
