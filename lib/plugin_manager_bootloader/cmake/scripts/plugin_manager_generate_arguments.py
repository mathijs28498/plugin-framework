import argparse
from pathlib import Path
from dataclasses import dataclass


@dataclass
class GenerateCmakeArguments:
    target_name: str
    build_platform: str
    build_dynamic_plugins: bool

    plugin_registry_toml: Path
    requested_plugins_toml: Path

    source_cmake: Path

    generated_include_dir: Path
    generated_plugin_manager_bootloader_generated_src: Path
    generated_cmake: Path
    generated_statically_resolved_plugins_json: Path

    @classmethod
    def from_args(cls, args: argparse.Namespace) -> "GenerateCmakeArguments":
        return cls(
            target_name=args.target_name,
            build_platform=args.build_platform,
            build_dynamic_plugins=args.build_dynamic_plugins,
            #
            plugin_registry_toml=args.plugin_registry_toml_path,
            requested_plugins_toml=args.requested_plugins_toml_path,
            #
            source_cmake=args.source_cmake_path,
            #
            generated_include_dir=args.generated_include_dir_path,
            generated_plugin_manager_bootloader_generated_src=args.generated_plugin_manager_bootloader_generated_src_path,
            generated_cmake=args.generated_cmake_path,
            generated_statically_resolved_plugins_json=args.generated_statically_resolved_plugins_json_path,
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
        "--requested-plugins-toml-path",
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
        "--generated-statically-resolved-plugins-json-path",
        required=True,
        type=Path,
    )

    args = parser.parse_args()

    return GenerateCmakeArguments.from_args(args)


@dataclass
class GenerateCCodeArguments:
    target_name: str
    build_platform: str
    build_dynamic_plugins: bool

    plugin_registry_toml: Path
    requested_plugins_toml: Path
    statically_resolved_plugins_json: Path

    source_plugin_manager_bootloader_generated_src: Path
    generated_plugin_manager_bootloader_generated_src: Path

    @classmethod
    def from_args(cls, args: argparse.Namespace) -> "GenerateCCodeArguments":
        return cls(
            target_name=args.target_name,
            build_platform=args.build_platform,
            build_dynamic_plugins=args.build_dynamic_plugins,
            #
            plugin_registry_toml=args.plugin_registry_toml_path,
            requested_plugins_toml=args.requested_plugins_toml_path,
            statically_resolved_plugins_json=args.statically_resolved_plugins_json_path,
            #
            source_plugin_manager_bootloader_generated_src=args.source_plugin_manager_bootloader_generated_src_path,
            generated_plugin_manager_bootloader_generated_src=args.generated_plugin_manager_bootloader_generated_src_path,
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
        "--requested-plugins-toml-path",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--statically-resolved-plugins-json-path",
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

    args = parser.parse_args()

    return GenerateCCodeArguments.from_args(args)


@dataclass
class GenerateRegisterIncArguments:
    build_dynamic: bool
    manifest_toml: Path
    source_plugin_register_inc: Path
    source_plugin_dependencies: Path
    generated_plugin_register_inc: Path
    generated_plugin_dependencies: Path

    @classmethod
    def from_args(cls, args: argparse.Namespace) -> "GenerateRegisterIncArguments":
        return cls(
            build_dynamic=args.build_dynamic,
            manifest_toml=args.manifest_toml_path,
            source_plugin_register_inc=args.source_plugin_register_inc_path,
            source_plugin_dependencies=args.source_plugin_dependencies_path,
            generated_plugin_register_inc=args.generated_plugin_register_inc_path,
            generated_plugin_dependencies=args.generated_plugin_dependencies_path,
        )


def parse_register_inc_arguments() -> GenerateRegisterIncArguments:
    parser = argparse.ArgumentParser(description="Generate plugin files")

    parser.add_argument(
        "--build-dynamic",
        action="store_true",
    )
    parser.add_argument(
        "--manifest-toml-path",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--source-plugin-register-inc-path",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--source-plugin-dependencies-path",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--generated-plugin-register-inc-path",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--generated-plugin-dependencies-path",
        required=True,
        type=Path,
    )

    args = parser.parse_args()

    return GenerateRegisterIncArguments.from_args(args)
