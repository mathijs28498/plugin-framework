from dataclasses import dataclass
from pathlib import Path
from typing import Optional


@dataclass
class PluginDependency:
    interface_name: str
    variable_name: str


@dataclass
class PluginManifest:
    target_name: str
    static_only: bool
    interface_name: str
    plugin_name: str
    dependencies: list[PluginDependency]
    get_interface_fn: str
    init_fn: Optional[str]
    shutdown_fn: Optional[str]
    source_path: Path
    module_path: Optional[Path]
    exported_declarations: list[str]


@dataclass
class PluginRegistryInterface:
    interface_name: str
    default_plugin_name: str


@dataclass
class PluginRegistry:
    plugin_manifests: list[PluginManifest]
    interfaces: list[PluginRegistryInterface]


@dataclass
class InterfaceDefinition:
    interface_name: str
    default_plugin_manifest: PluginManifest
    plugin_manifests: list[PluginManifest]


@dataclass
class RequestedPlugin:
    interface_name: str
    plugin_name: str


@dataclass
class PluginProviderDependency:
    interface_name: str
    set_fn: str


@dataclass
class PluginProvider:
    plugin_manifest: PluginManifest
    framework_declarations: list[str]
    dependencies: list[PluginProviderDependency]
    get_interface_fn_text: str
    init_fn_text: str
    shutdown_fn_text: str
    is_explicit: bool
    is_initialized: bool


@dataclass
class InterfaceInstance:
    pass