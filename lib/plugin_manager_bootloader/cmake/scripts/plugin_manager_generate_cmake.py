from plugin_manager_generate_templates import *
from plugin_manager_types import *
from plugin_manager_parse import *
from plugin_manager_generate_arguments import parse_cmake_arguments
from plugin_manager_plugin_resolver import create_static_plugin_providers
from plugin_manager_plugin_resolver import (
    ensure_core_plugins_requested,
    resolve_requested_plugin_providers,
    check_resolved_requested_plugin_providers,
    sort_plugin_providers,
)

RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH = 256


def main():
    arguments = parse_cmake_arguments()

    plugin_registry_dict = read_toml(arguments.plugin_registry_toml)
    requested_plugins_dict = read_toml(arguments.requested_plugins_toml)

    plugin_registry = parse_plugin_registry(
        plugin_registry_dict, arguments.build_platform
    )

    plugin_providers = create_static_plugin_providers(
        plugin_registry, arguments.build_dynamic_plugins
    )

    if not arguments.build_dynamic_plugins:
        requested_plugins = parse_requested_plugins(requested_plugins_dict)
        # Make sure the same plugin is not requested more than once as this is not supported
        requested_plugins.extend(
            ensure_core_plugins_requested(plugin_providers, requested_plugins)
        )

        requested_plugin_providers: list[PluginProvider] = []
        for _ in range(RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH):
            if not requested_plugins:
                break
            new_requested_plugin_providers = resolve_requested_plugin_providers(
                plugin_providers, requested_plugins
            )

            requested_plugins = check_resolved_requested_plugin_providers(
                new_requested_plugin_providers,
                requested_plugin_providers,
                requested_plugins,
            )

            requested_plugin_providers.extend(new_requested_plugin_providers)
        else:
            print(
                f"Hit maximum recursive dependency solver depth, maximum ({RECURSIVE_DEPENDENCY_SOLVER_MAX_DEPTH})"
            )
            return -1

        plugin_providers = requested_plugin_providers

    generate_statically_resolved_plugin_providers_json(
        arguments.generated_statically_resolved_plugins_json, plugin_providers
    )

    generate_plugin_manager_cmake(
        arguments.source_cmake,
        arguments.generated_cmake,
        arguments.target_name,
        [
            arguments.generated_include_dir,
        ],
        [
            arguments.generated_plugin_manager_bootloader_generated_src,
        ],
        plugin_providers,
    )


if __name__ == "__main__":
    main()
