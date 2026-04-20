import json
from collections.abc import Iterator
from collections import Counter
from typing import Any
import sys

from plugin_sdk_core.datatypes import PluginManifest, PluginDependency, PluginLifetime
from plugin_sdk_core.utils import read_toml
from plugin_sdk_core.parsers.manifest_parse import parse_plugin_manifest
from internal_core.datatypes import PluginRegistryInterface

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

from internal_core.datatypes import (
    PluginRegistry,
    AppConfig,
    RequestedPlugin,
)
from typing import Optional


def replace_variables_in_path_plugin_registry(path: str, plugin_registry_dir_path: Path) -> Path:
    return Path(
        path.replace("${plugin_registry}", plugin_registry_dir_path.as_posix())
    )


# TODO: Add error handling
def parse_plugin_registry(
    plugin_registry_dict: dict[str, Any],
    plugin_registry_dir_path: Path,
    build_platform: str,
) -> PluginRegistry:
    plugin_registry_dict["plugin_manifests"] = [
        replace_variables_in_path_plugin_registry(manifest_dir_path, plugin_registry_dir_path)
        for manifest_dir_path in plugin_registry_dict["plugin_manifests"]
    ]

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


def parse_app_dict(
    requested_plugins_dict: dict[str, Any],
) -> AppConfig:
    # TODO: Add error handling
    # TODO: Check if plugin name works properly here if not wanting default
    requested_plugins = [
        RequestedPlugin(
            interface_name=requested_plugin_interface_name,
            plugin_name=requested_plugin.get("plugin_name"),
            lifetime=PluginLifetime(requested_plugin.get("lifetime", "unknown")),
            is_explicit=True,
        )
        for requested_plugin_interface_name, requested_plugin in requested_plugins_dict.get(
            "requested_plugins", {}
        ).items()
    ]

    interface_counts = Counter(req.interface_name for req in requested_plugins)
    duplicates = [name for name, count in interface_counts.items() if count > 1]

    if duplicates:
        raise ValueError(
            f"Duplicate requested plugins detected: {', '.join(duplicates)}. "
            "Each interface can only be requested once."
        )

    return AppConfig(requested_plugins=requested_plugins)


def parse_lifetime(val: str) -> PluginLifetime:
    if val.startswith("PluginLifetime."):
        return PluginLifetime[val.split(".")[1]]
    return PluginLifetime(val)


def parse_statically_resolved_plugin_manifests(
    plugin_manifest_dicts: list[dict[str, Any]],
) -> Iterator[PluginManifest]:
    for plugin_manifest_dict in plugin_manifest_dicts:
        try:
            supported_lifetimes = [
                parse_lifetime(lifetime)
                for lifetime in plugin_manifest_dict["supported_lifetimes"]
            ]

            dependencies = {
                dependency_interface_name: PluginDependency(**dependency)
                for dependency_interface_name, dependency in plugin_manifest_dict[
                    "dependencies"
                ].items()
            }

            manifest_path = Path(plugin_manifest_dict["manifest_path"])
            source_path = Path(plugin_manifest_dict["source_path"])
            module_path = Path(plugin_manifest_dict["module_path"])

            plugin_manifest = PluginManifest(
                target_name=plugin_manifest_dict["target_name"],
                interface_name=plugin_manifest_dict["interface_name"],
                interface_version=plugin_manifest_dict["interface_version"],
                plugin_name=plugin_manifest_dict["plugin_name"],
                supported_lifetimes=supported_lifetimes,
                preferred_lifetime=parse_lifetime(
                    plugin_manifest_dict["preferred_lifetime"]
                ),
                has_init=plugin_manifest_dict["has_init"],
                has_shutdown=plugin_manifest_dict["has_shutdown"],
                dependencies=dependencies,
                manifest_path=manifest_path,
                source_path=source_path,
                module_path=module_path,
            )
        except KeyError:
            raise KeyError(
                f"Something went wrong parsing plugin manifest at: {plugin_manifest_dict}"
            )
        yield plugin_manifest
