from dataclasses import dataclass
from typing import Optional

from plugin_sdk_core.datatypes import PluginManifest, PluginLifetime


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
    lifetime: PluginLifetime
    is_explicit: bool


@dataclass
class AppConfig:
    requested_plugins: list[RequestedPlugin]
    max_plugin_amount: int
    memory_arena_size: int


