from internal_core.generators.generate_compile_files import (
    generate_plugin_manager_bootloader_generated_src,
    generate_plugin_manager_depfile,
)
from internal_core.parsers.plugin_manager_parse import (
    parse_app_dict,
    parse_plugin_registry,
    parse_statically_resolved_plugin_manifests,
)
from internal_core.plugin_resolver import create_interface_definitions

from plugin_sdk_core.utils import read_json, read_toml

import argparse
from pathlib import Path
from dataclasses import dataclass


@dataclass
class GenerateCCodeArguments:
    target_name: str
    build_platform: str
    build_dynamic_plugins: bool

    plugin_registry_toml: Path
    app_toml: Path
    statically_resolved_plugin_manifests_json: Path

    source_plugin_manager_bootloader_generated_src: Path
    generated_plugin_manager_bootloader_generated_src: Path

    depfile: Path

    @classmethod
    def from_args(cls, args: argparse.Namespace) -> "GenerateCCodeArguments":
        return cls(
            target_name=args.target_name,
            build_platform=args.build_platform,
            build_dynamic_plugins=args.build_dynamic_plugins,
            #
            plugin_registry_toml=args.plugin_registry_toml_path,
            app_toml=args.app_toml_path,
            statically_resolved_plugin_manifests_json=args.statically_resolved_plugin_manifests_json_path,
            #
            source_plugin_manager_bootloader_generated_src=args.source_plugin_manager_bootloader_generated_src_path,
            generated_plugin_manager_bootloader_generated_src=args.generated_plugin_manager_bootloader_generated_src_path,
            depfile=args.depfile_path,
        )


def parce_c_code_arguments() -> GenerateCCodeArguments:
    parser = argparse.ArgumentParser(description="Generate c code files")

    parser.add_argument(
        "--target-name",
        required=True,
    )
    parser.add_argument(
        "--build-platform",
        required=True,
    )
    parser.add_argument(
        "--build-dynamic-plugins",
        action="store_true",
    )

    parser.add_argument(
        "--plugin-registry-toml-path",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--app-toml-path",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--statically-resolved-plugin-manifests-json-path",
        required=True,
        type=Path,
    )

    parser.add_argument(
        "--source-plugin-manager-bootloader-generated-src-path",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--generated-plugin-manager-bootloader-generated-src-path",
        required=True,
        type=Path,
    )

    parser.add_argument(
        "--depfile-path",
        required=True,
        type=Path,
    )

    args = parser.parse_args()

    return GenerateCCodeArguments.from_args(args)


def main():
    arguments = parce_c_code_arguments()

    plugin_registry_dict = read_toml(arguments.plugin_registry_toml)
    app_dict = read_toml(arguments.app_toml)
    statically_resolved_plugin_manifests_dict = read_json(
        arguments.statically_resolved_plugin_manifests_json
    )

    plugin_registry = parse_plugin_registry(
        plugin_registry_dict,
        arguments.plugin_registry_toml.parent,
        arguments.build_platform,
    )

    app_config = parse_app_dict(app_dict)

    plugin_manifests = list(
        parse_statically_resolved_plugin_manifests(
            statically_resolved_plugin_manifests_dict
        )
    )

    interface_definitions = create_interface_definitions(plugin_registry)

    generate_plugin_manager_bootloader_generated_src(
        arguments.source_plugin_manager_bootloader_generated_src,
        arguments.generated_plugin_manager_bootloader_generated_src,
        interface_definitions,
        plugin_manifests,
        app_config.requested_plugins,
    )

    generate_plugin_manager_depfile(
        arguments.depfile,
        arguments.generated_plugin_manager_bootloader_generated_src,
        plugin_registry,
    )


if __name__ == "__main__":
    main()
