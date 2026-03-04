from plugin_manager_generate_templates import *
from plugin_manager_types import *
from plugin_manager_parse_json import *
from plugin_manager_generate_arguments import parse_cmake_arguments


def main():
    arguments = parse_cmake_arguments()

    plugin_registry_json = read_json(arguments.plugin_registry_json)

    internal_plugins = parse_internal_plugins(plugin_registry_json)

    generate_plugin_manager_cmake(
        arguments.source_cmake,
        arguments.generated_cmake,
        arguments.target_name,
        arguments.generated_get_setup_context_src,
        arguments.generated_plugin_registry_src,
        internal_plugins,
    )


if __name__ == "__main__":
    main()
