from dataclasses import dataclass
from pathlib import Path
from typing import Optional
from enum import Enum


@dataclass
class PluginDependency:
    variable_name: str
    is_deferred: bool

class PluginLifetime(Enum):
    UNKNOWN = "unknown"
    SINGLETON = "singleton"
    SCOPED = "scoped"
    TRANSIENT = "transient"

@dataclass
class PluginManifest:
    target_name: str
    interface_name: str
    plugin_name: str
    supported_lifetimes: list[PluginLifetime]
    preferred_lifetime: PluginLifetime
    has_init: bool
    has_shutdown: bool
    dependencies: dict[str, PluginDependency]
    source_path: Path
    module_path: Path


@dataclass
class PluginRegistryInterface:
    interface_name: str
    default_plugin_name: str


@dataclass
class PluginRegistry:
    plugin_manifests: list[PluginManifest]
    # TODO: Change this name to defaults, as thats what they actually are
    interfaces: list[PluginRegistryInterface]


@dataclass
class InterfaceDefinition:
    interface_name: str
    default_plugin_manifest: PluginManifest
    plugin_manifests: list[PluginManifest]


@dataclass
class RequestedPlugin:
    interface_name: str
    plugin_name: Optional[str]
    is_explicit: bool


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
