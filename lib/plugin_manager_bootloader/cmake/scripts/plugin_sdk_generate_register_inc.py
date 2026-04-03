from plugin_manager_generate_arguments import parse_register_inc_arguments
from plugin_manager_parse import parse_plugin_manifest, read_toml
from plugin_manager_generate_templates import (
    generate_register_inc,
    generate_plugin_dependencies,
)

from pathlib import Path


def main():
    arguments = parse_register_inc_arguments()

    plugin_manifest_dict = read_toml(arguments.manifest_toml)
    plugin_manifest = parse_plugin_manifest(
        plugin_manifest_dict, arguments.manifest_toml
    )

    # TODO: Change names to have header in the naming
    generate_plugin_dependencies(
        arguments.source_plugin_dependencies,
        arguments.generated_plugin_dependencies,
        plugin_manifest,
    )

    generate_register_inc(
        arguments.source_plugin_register_inc,
        arguments.generated_plugin_register_inc,
        arguments.build_dynamic,
        plugin_manifest,
    )


if __name__ == "__main__":
    main()
