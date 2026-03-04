from dataclasses import dataclass
from pathlib import Path


@dataclass
class PluginSourceData:
    target_name: str
    source_path: Path
    register_path: Path


@dataclass
class PluginDependency:
    pass


@dataclass
class InternalPlugin:
    source_data: PluginSourceData
    forward_declarations: list[str]
    interface_name: str
    plugin_name: str
    dependencies: list[PluginDependency]
    get_interface_fn_text: str
    init_fn_text: str
    shutdown_fn_text: str
    is_explicit: bool
    is_initialized: bool


@dataclass
class RequestedPlugin:
    interface_name: str
    plugin_name: str
    is_explicit: bool
    is_resolved: bool


@dataclass
class PluginProvider:
    pass


@dataclass
class PluginListData:
    requested_plugins: list[RequestedPlugin]
    sorted_plugin_providers_indices: list[int]
    plugin_providers: list[PluginProvider]


@dataclass
class PluginDefinition:
    plugin_name: str
    module_path: Path
    source_path: Path


@dataclass
class InterfaceDefinition:
    interface_name: str
    default_plugin_name: str
    plugin_definitions: list[PluginDefinition]
