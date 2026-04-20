from plugin_sdk_core.datatypes import PluginManifest, PluginLifetime, PluginDependency
from typing import Any, Optional
from pathlib import Path

# TODO: Fix this better
build_path = "../../build/bin/Debug"

def replace_variables_in_path_plugin_manifest(path: str) -> Path:
    return Path(
        path.replace("${build_dir}", build_path)
    ).resolve()

def parse_plugin_manifest(
    manifest_dict: dict[str, Any], manifest_dir_path: Path
) -> PluginManifest:
    try:
        interface_name: str = manifest_dict["interface_name"]
        interface_version: str = manifest_dict["interface_version"]
        plugin_name: str = manifest_dict["plugin_name"]
        target_name: str = f"{interface_name}_{plugin_name}"

        source_path_str: Optional[str] = manifest_dict.get("source_path")
        module_path_str: Optional[str] = manifest_dict.get("module_path")

        supported_lifetimes = [
            PluginLifetime(lifetime)
            for lifetime in manifest_dict["supported_lifetimes"]
        ]
        preferred_lifetime: PluginLifetime = manifest_dict.get(
            "preferred_lifetime", PluginLifetime.UNKNOWN
        )

        has_init: bool = manifest_dict.get("init", False)
        has_shutdown: bool = manifest_dict.get("shutdown", False)

        dependencies: dict[str, PluginDependency] = {}
        for dependency_interface_name, dependency_config_dict in manifest_dict.get(
            "dependencies", {}
        ).items():
            if dependency_config_dict is None:
                dependency_config_dict = {}

            dependency_variable_name = dependency_config_dict.get(
                "variable_name", dependency_interface_name
            )
            dependency_is_deferred = dependency_config_dict.get("deferred", False)
            if not has_init:
                dependency_is_deferred = True

            dependencies[dependency_interface_name] = PluginDependency(
                variable_name=dependency_variable_name,
                is_deferred=dependency_is_deferred,
            )

        source_path: Path = (
            Path(source_path_str) if source_path_str else manifest_dir_path
        )
        # TODO: Add default module path maybe
        module_path: Path = (
            Path(replace_variables_in_path_plugin_manifest(module_path_str)) if module_path_str else Path("Add default here")
        )
    except TypeError:
        raise TypeError(
            f"Something went wrong parsing plugin manifest at: {manifest_dir_path}"
        )

    return PluginManifest(
        target_name=target_name,
        interface_name=interface_name,
        interface_version=interface_version,
        plugin_name=plugin_name,
        supported_lifetimes=supported_lifetimes,
        preferred_lifetime=preferred_lifetime,
        has_init=has_init,
        has_shutdown=has_shutdown,
        dependencies=dependencies,
        manifest_path=manifest_dir_path / "manifest.toml",
        source_path=source_path,
        module_path=module_path,
    )
