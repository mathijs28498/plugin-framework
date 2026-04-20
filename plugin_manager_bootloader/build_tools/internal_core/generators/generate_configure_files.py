from pathlib import Path
from dataclasses import asdict
import json
import textwrap

from plugin_sdk_core.utils import create_and_write_to_file, configure_file, indent_prefix
from plugin_sdk_core.datatypes import PluginManifest


def generate_statically_resolved_plugin_manifests_json(
    destination_path: Path,
    plugin_manifests: list[PluginManifest],
):
    plugin_providers_dict = [
        asdict(plugin_manifest) for plugin_manifest in plugin_manifests
    ]

    content = json.dumps(plugin_providers_dict, indent=4, default=str)
    create_and_write_to_file(destination_path, content, False)


def generate_plugin_manager_cmake(
    source_path: Path,
    destination_path: Path,
    target_name: str,
    include_paths: list[Path],
    source_paths: list[Path],
    plugin_manifests: list[PluginManifest],
):
    # TODO: Add loops in the template
    add_subdirectory_text = "\n\n".join(
        textwrap.dedent(
            f"""\
            add_subdirectory(
                "{plugin_manifest.source_path.as_posix()}"
                "${{CMAKE_CURRENT_BINARY_DIR}}/generated_plugins/{plugin_manifest.target_name}"
            )"""
        )
        for plugin_manifest in plugin_manifests
    )

    plugin_target_cmake_list = f"\n{indent_prefix * 1}".join(
        plugin_manifest.target_name
        for plugin_manifest in plugin_manifests
    )

    generated_include_directories_text = f"\n{indent_prefix * 1}".join(
        f'"{include_path.as_posix()}"' for include_path in include_paths
    )

    generated_target_sources_text = f"\n{indent_prefix * 1}".join(
        f'"{source_path.as_posix()}"' for source_path in source_paths
    )

    replacements = [
        ("TARGET_NAME", target_name),
        ("ADD_SUBDIRECTORY_TEXT", add_subdirectory_text),
        ("PLUGIN_TARGET_CMAKE_LIST", plugin_target_cmake_list),
        ("GENERATED_INCLUDE_DIRECTORIES_TEXT", generated_include_directories_text),
        ("GENERATED_TARGET_SOURCES_TEXT", generated_target_sources_text),
    ]

    configure_file(source_path, destination_path, replacements, True)
