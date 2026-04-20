from internal_core.generators.generate_configure_files import (
    generate_statically_resolved_plugin_manifests_json,
    generate_plugin_manager_cmake,
)
from internal_core.parsers.plugin_manager_parse import (
    parse_plugin_registry,
    parse_app_dict,
)
from internal_core.plugin_resolver import (
    resolve_compile_time_plugins,
    sort_plugin_manifests,
)

from plugin_sdk_core.utils import read_toml
from plugin_sdk_core.datatypes import PluginManifest

from pathlib import Path
from dataclasses import dataclass
import argparse


@dataclass
class GenerateCmakeArguments:
    target_name: str
    build_platform: str
    build_dynamic_plugins: bool

    plugin_registry_toml: Path
    app_toml: Path

    source_cmake: Path

    generated_include_dir: Path
    generated_plugin_manager_bootloader_generated_src: Path
    generated_cmake: Path
    generated_statically_resolved_plugin_manifests_json: Path

    @classmethod
    def from_args(cls, args: argparse.Namespace) -> "GenerateCmakeArguments":
        return cls(
            target_name=args.target_name,
            build_platform=args.build_platform,
            build_dynamic_plugins=args.build_dynamic_plugins,
            #
            plugin_registry_toml=args.plugin_registry_toml_path,
            app_toml=args.app_toml_path,
            #
            source_cmake=args.source_cmake_path,
            #
            generated_include_dir=args.generated_include_dir_path,
            generated_plugin_manager_bootloader_generated_src=args.generated_plugin_manager_bootloader_generated_src_path,
            generated_cmake=args.generated_cmake_path,
            generated_statically_resolved_plugin_manifests_json=args.generated_statically_resolved_plugin_manifests_json_path,
        )


def parse_cmake_arguments() -> GenerateCmakeArguments:
    parser = argparse.ArgumentParser(description="Generate cmake files")

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
        "--source-cmake-path",
        required=True,
        type=Path,
    )

    parser.add_argument(
        "--generated-include-dir-path",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--generated-plugin-manager-bootloader-generated-src-path",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--generated-cmake-path",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--generated-statically-resolved-plugin-manifests-json-path",
        required=True,
        type=Path,
    )

    args = parser.parse_args()

    return GenerateCmakeArguments.from_args(args)


# TODO: Look into static and dynamic loaded at same time
def main():
    arguments = parse_cmake_arguments()

    plugin_registry_dict = read_toml(arguments.plugin_registry_toml)

    plugin_registry = parse_plugin_registry(
        plugin_registry_dict,
        arguments.plugin_registry_toml.parent,
        arguments.build_platform,
    )

    plugin_manifests: list[PluginManifest] = []

    if not arguments.build_dynamic_plugins:
        app_dict = read_toml(arguments.app_toml)
        app_config = parse_app_dict(app_dict)
        plugin_manifests = resolve_compile_time_plugins(app_config, plugin_registry)

    plugin_manifests = sort_plugin_manifests(plugin_manifests)

    generate_statically_resolved_plugin_manifests_json(
        arguments.generated_statically_resolved_plugin_manifests_json, plugin_manifests
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
        plugin_manifests,
    )


if __name__ == "__main__":
    main()
