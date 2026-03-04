import argparse
from pathlib import Path
from dataclasses import dataclass


@dataclass
class GenerateCmakeArguments:
    target_name: str
    plugin_registry_json: Path
    source_cmake: Path
    generated_plugin_registry_src: Path
    generated_get_setup_context_src: Path
    generated_cmake: Path

    @classmethod
    def from_args(cls, args: argparse.Namespace) -> "GenerateCmakeArguments":
        return cls(
            target_name=args.target_name,
            plugin_registry_json=args.plugin_registry_json_path,
            source_cmake=args.source_cmake_path,
            generated_plugin_registry_src=args.generated_plugin_registry_src_path,
            generated_get_setup_context_src=args.generated_get_setup_context_src_path,
            generated_cmake=args.generated_cmake_path,
        )


def parse_cmake_arguments() -> GenerateCmakeArguments:
    parser = argparse.ArgumentParser(description="Generate cmake files")

    parser.add_argument("--target-name", required=True)
    parser.add_argument("--plugin-registry-json-path", required=True, type=Path)
    parser.add_argument("--source-cmake-path", required=True, type=Path)
    parser.add_argument(
        "--generated-plugin-registry-src-path", required=True, type=Path
    )
    parser.add_argument(
        "--generated-get-setup-context-src-path", required=True, type=Path
    )
    parser.add_argument("--generated-cmake-path", required=True, type=Path)

    args = parser.parse_args()

    return GenerateCmakeArguments.from_args(args)


@dataclass
class GenerateCCodeArguments:
    target_name: str
    build_platform: str

    plugin_registry_json: Path
    plugin_list_json: Path

    source_plugin_registry_header: Path
    source_plugin_manager_header: Path
    source_plugin_registry_src: Path
    source_get_setup_context_src: Path

    generated_plugin_registry_header: Path
    generated_plugin_manager_header: Path
    generated_plugin_registry_src: Path
    generated_get_setup_context_src: Path

    @classmethod
    def from_args(cls, args: argparse.Namespace) -> "GenerateCCodeArguments":
        return cls(
            target_name=args.target_name,
            build_platform=args.build_platform,

            plugin_registry_json=args.plugin_registry_json_path,
            plugin_list_json=args.plugin_list_json_path,

            source_plugin_registry_header=args.source_plugin_registry_header_path,
            source_plugin_manager_header=args.source_plugin_manager_header_path,
            source_plugin_registry_src=args.source_plugin_registry_src_path,
            source_get_setup_context_src=args.source_get_setup_context_src_path,

            generated_plugin_registry_header=args.generated_plugin_registry_header_path,
            generated_plugin_manager_header=args.generated_plugin_manager_header_path,
            generated_plugin_registry_src=args.generated_plugin_registry_src_path,
            generated_get_setup_context_src=args.generated_get_setup_context_src_path,
        )


def parce_c_code_arguments() -> GenerateCCodeArguments:
    parser = argparse.ArgumentParser(description="Generate c code files")

    parser.add_argument("--target-name", required=True)
    parser.add_argument("--build-platform", required=True)

    parser.add_argument("--plugin-registry-json-path", required=True, type=Path)
    parser.add_argument("--plugin-list-json-path", required=True, type=Path)

    parser.add_argument(
        "--source-plugin-registry-header-path", required=True, type=Path
    )
    parser.add_argument("--source-plugin-manager-header-path", required=True, type=Path)
    parser.add_argument("--source-plugin-registry-src-path", required=True, type=Path)
    parser.add_argument("--source-get-setup-context-src-path", required=True, type=Path)

    parser.add_argument(
        "--generated-plugin-registry-header-path", required=True, type=Path
    )
    parser.add_argument(
        "--generated-plugin-manager-header-path", required=True, type=Path
    )
    parser.add_argument(
        "--generated-plugin-registry-src-path", required=True, type=Path
    )
    parser.add_argument(
        "--generated-get-setup-context-src-path", required=True, type=Path
    )

    args = parser.parse_args()

    return GenerateCCodeArguments.from_args(args)
