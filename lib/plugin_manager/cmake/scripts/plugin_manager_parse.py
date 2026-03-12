import json
from collections.abc import Iterator
from collections import Counter
from typing import Any
import sys

# TODO: Check if this properly works for older versions
if sys.version_info >= (3, 11):
    import tomllib
else:
    import importlib.util

    if importlib.util.find_spec("tomli") is None:
        print(
            "Error: Your Python version is older than 3.11.\n"
            "To parse plugin manifests, you must either:\n"
            "  1. Upgrade to Python 3.11 or newer (Recommended)\n"
            "  2. Install the fallback library by running: pip install tomli",
            file=sys.stderr,
        )
        sys.exit(1)

    import tomli as tomllib

import re
from pathlib import Path

from plugin_manager_types import *


# TODO: Add error handling
def parse_plugin_registry(
    plugin_registry_dict: dict[str, Any], build_platform: str
) -> PluginRegistry:

    plugin_manifests = [
        parse_plugin_manifest(
            read_toml(Path(manifest_dir_path) / "manifest.toml"),
            Path(manifest_dir_path),
        )
        for manifest_dir_path in plugin_registry_dict["plugin_manifests"]
    ]

    interface_definitions = []
    interface_definitions_dict = plugin_registry_dict["interfaces"]
    for interface_definition_dict in interface_definitions_dict:
        interface_name = interface_definition_dict["interface_name"]

        # TODO: Create a custom parsing interface error. For errors of [] fields, add a try except and create a custom error at the end
        default_plugin_obj = interface_definition_dict.get("default")
        if default_plugin_obj is None:
            raise ValueError(
                f"Error parsing interface '{interface_name}': Missing required 'default' key. A default plugin must be specified."
            )

        if isinstance(default_plugin_obj, str):
            default_plugin_name = default_plugin_obj
        elif isinstance(default_plugin_obj, list):
            if not default_plugin_obj:
                raise ValueError(
                    f"Error parsing interface '{interface_name}': The 'default' list is empty. Please provide at least one plugin default."
                )

            for default_plugin_info_dict in default_plugin_obj:
                target = default_plugin_info_dict["target"]

                # TODO Add other platforms
                if target == "win32" and build_platform == "win32":
                    default_plugin_name = default_plugin_info_dict["plugin_name"]
                    break
            else:
                raise ValueError(
                    f"Error parsing interface '{interface_name}': No default plugin found for build platform '{build_platform}'."
                )
        else:
            raise ValueError(
                f"Error parsing interface '{interface_name}': Invalid type for 'default'. Expected a string or a list, but got {type(default_plugin_obj).__name__}."
            )

        interface_definitions.append(
            PluginRegistryInterface(
                interface_name=interface_name,
                default_plugin_name=default_plugin_name,
            )
        )

    return PluginRegistry(
        plugin_manifests=plugin_manifests,
        interfaces=interface_definitions,
    )


# TODO: Add more sane defaults if necessary
def parse_plugin_manifest(
    manifest_dict: dict[str, Any], manifest_dir_path: Path
) -> PluginManifest:

    interface_name: str = manifest_dict["interface_name"]
    plugin_name: str = manifest_dict["plugin_name"]
    source_path_str: Optional[str] = manifest_dict.get("source_path")
    module_path_str: Optional[str] = manifest_dict.get("module_path")

    dependencies: list[PluginDependency] = []
    for dependency_dict in manifest_dict.get("dependencies", []):
        dependency_interface_name = dependency_dict["interface_name"]
        dependencies.append(
            PluginDependency(
                interface_name=dependency_interface_name,
                variable_name=dependency_dict.get(
                    "variable_name", dependency_interface_name
                ),
            )
        )

    return PluginManifest(
        target_name=f"{interface_name}_{plugin_name}",
        static_only=manifest_dict.get("static_only", False),
        core=manifest_dict.get("core", False),
        interface_name=interface_name,
        plugin_name=plugin_name,
        dependencies=dependencies,
        get_interface_fn=manifest_dict["get_interface"],
        init_fn=manifest_dict.get("init"),
        shutdown_fn=manifest_dict.get("shutdown"),
        source_path=Path(source_path_str) if source_path_str else manifest_dir_path,
        module_path=Path(module_path_str) if module_path_str else None,
        exported_declarations=manifest_dict.get("exported_declarations", []),
    )


def parse_requested_plugins(requested_plugins_dict: dict[str, Any]) -> list[RequestedPlugin]:
    # TODO: Add error handling
    requested_plugins = [
        RequestedPlugin(
            interface_name=requested_plugin["interface_name"],
            plugin_name=requested_plugin.get("plugin_name", ""),
            is_explicit=True,
        )
        for requested_plugin in requested_plugins_dict["plugins"]
    ]

    interface_counts = Counter(req.interface_name for req in requested_plugins)
    duplicates = [name for name, count in interface_counts.items() if count > 1]

    if duplicates:
        raise ValueError(
            f"Duplicate requested plugins detected: {', '.join(duplicates)}. "
            "Each interface can only be requested once."
        )

    return requested_plugins


def parse_statically_resolved_plugins(
    statically_resolved_plugins_dict: list[dict[str, Any]],
) -> Iterator[PluginProvider]:
    for plugin_provider_dict in statically_resolved_plugins_dict:
        plugin_manifest_dict = plugin_provider_dict["plugin_manifest"]
        plugin_manifest_dict["source_path"] = Path(plugin_manifest_dict["source_path"])
        if plugin_manifest_dict.get("module_path"):
            plugin_manifest_dict["module_path"] = Path(
                plugin_manifest_dict["module_path"]
            )

        plugin_manifest_dict["dependencies"] = [
            PluginDependency(**dependency)
            for dependency in plugin_manifest_dict["dependencies"]
        ]

        plugin_manifest = PluginManifest(**plugin_manifest_dict)

        plugin_provider_dependencies = [
            PluginProviderDependency(**dependency)
            for dependency in plugin_provider_dict["dependencies"]
        ]

        plugin_provider_dict["plugin_manifest"] = plugin_manifest
        plugin_provider_dict["dependencies"] = plugin_provider_dependencies

        yield PluginProvider(**plugin_provider_dict)


def read_json(source_path: Path):
    print(f"-- Reading: {source_path}")
    with open(source_path, "r", encoding="utf-8") as f:
        return json.load(f)


def read_toml(source_path: Path):
    print(f"-- Reading: {source_path}")
    with open(source_path, "rb") as f:
        return tomllib.load(f)
